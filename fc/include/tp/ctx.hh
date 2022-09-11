#ifndef CTX_HH
#define CTX_HH
#include "c++.h"
#include "fcontext.hpp"
#include "fixedsize_stack.hh"
#include <ostream>
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#include <tuple>
#include <type_traits>
#if defined _MSC_VER
#if defined(_M_X64)
#  pragma pack(push,16)
#else
#  pragma pack(push,8)
#endif
# pragma warning(push)
//# pragma warning(disable: 4702)
#elif defined __CODEGEARC__
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#elif defined __BORLANDC__
#if __BORLANDC__ != 0x600
#pragma nopushoptwarn
#  pragma option push -a8 -Vx- -Ve- -b- -pc -Vmv -VC- -Vl- -w-8027 -w-8026
#endif
#endif
namespace ctx {
  struct forced_unwind {
	fcontext_t fctx{ nullptr }; forced_unwind() = default; forced_unwind(fcontext_t fctx_): fctx(fctx_) {}
  };
  inline transfer_t context_unwind(transfer_t t) { throw forced_unwind(t.fctx); return { nullptr, nullptr }; }
  template< typename Rec >
  transfer_t context_exit(transfer_t t) noexcept {
	Rec* rec = static_cast<Rec*>(t.data); rec->deallocate(); return { nullptr, nullptr }; // destroy context stack
  }
  struct preallocated {
	void* sp; size_t size; stack_context sctx;
	preallocated(void* sp_, size_t size_, stack_context sctx_) noexcept: sp(sp_), size(size_), sctx(sctx_) {}
  };
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
  template< typename Record, typename Fn >
  fcontext_t create_context1(fixedsize_stack&& salloc, Fn&& fn) {
	auto sctx = salloc.allocate();
	// reserve space for control structure
	void* storage = reinterpret_cast<void*>(
			(reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sizeof(Record)))
			& ~static_cast<uintptr_t>(0xff));
	// placment new for control structure on context stack
	Record* record = new (storage) Record{
			sctx, std::forward< fixedsize_stack >(salloc), std::forward< Fn >(fn) };
	// 64byte gab between control structure and stack top
	// should be 16byte aligned
	void* stack_top = reinterpret_cast<void*>(
			reinterpret_cast<uintptr_t>(storage) - static_cast<uintptr_t>(64));
	void* stack_bottom = reinterpret_cast<void*>(
			reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
	// create fast-context
	const size_t size = reinterpret_cast<uintptr_t>(stack_top) - reinterpret_cast<uintptr_t>(stack_bottom);
	const fcontext_t fctx = make_fcontext(stack_top, size, &context_entry< Record >);
	assert(nullptr != fctx); // transfer control structure to context-stack
	return jump_fcontext(fctx, record).fctx;
  }
  template< typename Record, typename Fn >
  fcontext_t create_context2(preallocated palloc, fixedsize_stack&& salloc, Fn&& fn) {
	// reserve space for control structure
	void* storage = reinterpret_cast<void*>(
			(reinterpret_cast<uintptr_t>(palloc.sp) - static_cast<uintptr_t>(sizeof(Record)))
			& ~static_cast<uintptr_t>(0xff));
	// placment new for control structure on context-stack
	Record* record = new (storage) Record{
			palloc.sctx, std::forward< fixedsize_stack >(salloc), std::forward< Fn >(fn) };
	// 64byte gab between control structure and stack top
	void* stack_top = reinterpret_cast<void*>(
			reinterpret_cast<uintptr_t>(storage) - static_cast<uintptr_t>(64));
	void* stack_bottom = reinterpret_cast<void*>(
			reinterpret_cast<uintptr_t>(palloc.sctx.sp) - static_cast<uintptr_t>(palloc.sctx.size));
	// create fast-context
	const size_t size = reinterpret_cast<uintptr_t>(stack_top) - reinterpret_cast<uintptr_t>(stack_bottom);
	const fcontext_t fctx = make_fcontext(stack_top, size, &context_entry< Record >);
	assert(nullptr != fctx); // transfer control structure to context-stack
	return jump_fcontext(fctx, record).fctx;
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
	template<typename Fn>
	friend continuation callcc(fixedsize_stack&&, Fn&&);
	template<typename Fn>
	friend continuation callcc(preallocated, fixedsize_stack&&, Fn&&);
  public:
	continuation(fcontext_t fctx) noexcept: fctx_{ fctx } {}
	continuation() noexcept = default;
	
	template<typename Fn, typename = std::disable_overload< continuation, Fn >>
	continuation(Fn&& fn): continuation{ std::allocator_arg, fixedsize_stack(stack_traits::minimum_size()), std::forward< Fn >(fn) } {}
	template<typename Fn>
	continuation(std::allocator_arg_t, fixedsize_stack&& salloc, Fn&& fn) :
	  fctx_{ create_context1< record< continuation, Fn > >(
			  std::forward< fixedsize_stack >(salloc), std::forward< Fn >(fn)) } {}
	template<typename Fn>
	continuation(std::allocator_arg_t, preallocated palloc, fixedsize_stack&& salloc, Fn&& fn) :
	  fctx_{ create_context2< record< continuation, Fn > >(
			  palloc, std::forward< fixedsize_stack >(salloc), std::forward< Fn >(fn)) } {}
    //fiber
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
	continuation yield()& { return std::move(*this).resume(); }
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
  template<typename Fn>
  continuation callcc(Fn&& fn, fixedsize_stack&& s = fixedsize_stack()) { return callcc(s, std::forward<Fn>(fn)); };
  template<typename Fn>
  continuation callcc(fixedsize_stack&& salloc, Fn&& fn) {
	return continuation{ create_context1<record<continuation, Fn>>(std::forward<fixedsize_stack>(salloc), std::forward< Fn >(fn)) }.resume();
  }
  template<typename Fn>
  continuation callcc(preallocated palloc, fixedsize_stack&& salloc, Fn&& fn) {
	return continuation{ create_context2<record<continuation, Fn>>(palloc, std::forward<fixedsize_stack>(salloc), std::forward<Fn>(fn)) }.resume();
  };
  typedef continuation fiber;
}
namespace fc {
  typedef ctx::continuation co;
}
#if defined _MSC_VER
# pragma warning(pop)
#pragma pack(pop)
#elif defined __CODEGEARC__
#  pragma option pop
#pragma nopushoptwarn
#elif defined __BORLANDC__
#if __BORLANDC__ != 0x600
#  pragma option pop
#pragma nopushoptwarn
#endif
#endif
#endif