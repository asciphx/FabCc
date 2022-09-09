#ifndef FCONTEXT_HH
#define FCONTEXT_HH
#include "fcontext.hpp"
#include "fixedsize_stack.hh"
#include <ostream>
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#if (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
#define BOOST_UNLIKELY(x) __builtin_expect(x, 0)
#define BOOST_LIKELY(x) __builtin_expect(x, 1)
#else
#define BOOST_UNLIKELY(x) x
#define BOOST_LIKELY(x) x
#endif
namespace context {
  struct forced_unwind {
	fcontext_t fctx{ nullptr }; forced_unwind() = default; forced_unwind(fcontext_t fctx_): fctx(fctx_) {}
  };
  inline transfer_t context_unwind(transfer_t t) { throw forced_unwind(t.fctx); return { nullptr, nullptr }; }
  template< typename Rec >
  transfer_t context_exit(transfer_t t) noexcept {
	Rec* rec = static_cast<Rec*>(t.data); rec->deallocate(); return { nullptr, nullptr }; // destroy context stack
  }
  template< typename Rec >
  void context_entry(transfer_t t) noexcept {
	// transfer control structure to the context-stack
	Rec* rec = static_cast<Rec*>(t.data); assert(nullptr != t.fctx); assert(nullptr != rec);
	try {// jump back to `create_context()`
	  t = jump_fcontext(t.fctx, nullptr); t.fctx = rec->run(t.fctx);// start executing
	} catch (forced_unwind const& ex) { t = { ex.fctx, nullptr }; }
	assert(nullptr != t.fctx); // destroy context-stack of `this`context on next context
	ontop_fcontext(t.fctx, rec, context_exit< Rec >);
  }
  template< typename Ctx, typename Fn >
  transfer_t context_ontop(transfer_t t) {
	auto p = static_cast<std::tuple< Fn > *>(t.data); assert(nullptr != p);
	typename std::decay< Fn >::type fn = std::get< 0 >(*p); t.data = nullptr; Ctx c{ t.fctx };
	// execute function, pass continuation via reference
	c = fn(std::move(c)); return { std::exchange(c.fctx_, nullptr), nullptr };
  }
  template< typename Ctx, typename Fn >
  class record {
  private:
	stack_context                                   sctx_;
	typename std::decay< fixedsize_stack >::type    salloc_;
	typename std::decay< Fn >::type                 fn_;
	static void destroy(record* p) noexcept {
	  typename std::decay< fixedsize_stack >::type salloc = std::move(p->salloc_);
	  stack_context sctx = p->sctx_; p->~record();// deallocate record
	  salloc.deallocate(sctx);// destroy stack with stack allocator
	}
  public:
	record(stack_context sctx, fixedsize_stack&& salloc, Fn&& fn) noexcept:
	  sctx_(sctx), salloc_(std::forward< fixedsize_stack >(salloc)), fn_(std::forward< Fn >(fn)) {}
	record(record const&) = delete; record& operator=(record const&) = delete;
	void deallocate() noexcept { destroy(this); }
	fcontext_t run(fcontext_t fctx) {
	  Ctx c{ fctx }; c = std::invoke(fn_, std::move(c)); return std::exchange(c.fctx_, nullptr);
	}
  };
  class continuation {
  private:
	template<typename Ctx, typename Fn> friend class record;
	template<typename Ctx, typename Fn> friend transfer_t context_ontop(transfer_t);
	fcontext_t  fctx_{ nullptr };
	continuation(fcontext_t fctx) noexcept: fctx_{ fctx } {}
  public:
	continuation() noexcept = default;
	~continuation() {
	  if (BOOST_UNLIKELY(nullptr != fctx_)) {
		ontop_fcontext(std::exchange(fctx_, nullptr), nullptr, context_unwind);
	  }
	}
	continuation(continuation&& other) noexcept { swap(other); }
	continuation& operator=(continuation&& other) noexcept {
	  if (BOOST_LIKELY(this != &other)) { continuation tmp = std::move(other); swap(tmp); } return *this;
	}
	continuation(continuation const& other) noexcept = delete;
	continuation& operator=(continuation const& other) noexcept = delete;
	continuation resume()& { return std::move(*this).resume(); }
	continuation resume()&& {
	  assert(nullptr != fctx_); return { jump_fcontext(std::exchange(fctx_, nullptr), nullptr).fctx };
	}
	template< typename Fn >
	continuation resume_with(Fn&& fn)& { return std::move(*this).resume_with(std::forward< Fn >(fn)); }
	template< typename Fn >
	continuation resume_with(Fn&& fn)&& {
	  assert(nullptr != fctx_); auto p = std::make_tuple(std::forward< Fn >(fn));
	  return { ontop_fcontext(std::exchange(fctx_, nullptr), &p, context_ontop< continuation, Fn >).fctx };
	}
	explicit operator bool() const noexcept { return nullptr != fctx_; }
	bool operator!() const noexcept { return nullptr == fctx_; }
	bool operator<(continuation const& other) const noexcept { return fctx_ < other.fctx_; }
	template< typename charT, class traitsT >
	friend std::basic_ostream< charT, traitsT >&
	  operator<<(std::basic_ostream< charT, traitsT >& os, continuation const& other) {
	  if (nullptr != other.fctx_) { return os << other.fctx_; } else { return os << "{not-a-context}"; }
	}
	void swap(continuation& other) noexcept { std::swap(fctx_, other.fctx_); }
  };
}
#endif