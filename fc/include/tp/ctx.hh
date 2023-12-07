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
#include <iostream>
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
    fcontext_t fctx{ nullptr }; forced_unwind() = default; forced_unwind(fcontext_t fctx_) : fctx(fctx_) {}
  };
  inline transfer_t ctx_unwind(transfer_t t) { throw forced_unwind(t.fctx); return { nullptr, nullptr }; }
  template< typename Rec >
  transfer_t ctx_exit(transfer_t t) noexcept {
    Rec* rec = static_cast<Rec*>(t.data); rec->deallocate(); return { nullptr, nullptr }; // destroy context stack
  }
  template< typename Rec >
  void ctx_entry(transfer_t t) noexcept {
    // transfer control structure to the context-stack
    Rec* rec = static_cast<Rec*>(t.data); assert(nullptr != t.fctx); assert(nullptr != rec);
    try {// jump back to `add_ctx()`
      t = jump_fcontext(t.fctx, nullptr); t.fctx = rec->run(t.fctx);// start executing
    }
    catch (forced_unwind const& ex) { t = { ex.fctx, nullptr }; }
    assert(nullptr != t.fctx); // destroy context-stack of `this`context on next context
    ontop_fcontext(t.fctx, rec, ctx_exit< Rec >);
  }
  template< typename Ctx, typename Fn >
  transfer_t ctx_ontop(transfer_t t) {
    auto p = static_cast<std::tuple< Fn > *>(t.data); assert(nullptr != p);
    typename std::decay< Fn >::type fn = std::get< 0 >(*p); t.data = nullptr; Ctx c{ t.fctx };
    // execute function, pass continuation via reference
    c = fn(std::move(c)); return { std::exchange(c.fctx_, nullptr), nullptr };
  }
  template< typename Record, typename Fn >
  fcontext_t add_ctx(fixedsize_stack&& salloc, Fn&& fn) {
    auto sctx = salloc.allocate();
    // reserve space for control structure控制结构预留空间
    void* storage = reinterpret_cast<void*>(
      (reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sizeof(Record)))
      & ~static_cast<uintptr_t>(0xff));
    // placment new for control structure on context stack在上下文堆栈上放置新的控件结构
    Record* record = new (storage) Record{
        sctx, std::forward< fixedsize_stack >(salloc), std::forward< Fn >(fn) };
    // 64byte gab between control structure and stack top控制结构和栈顶之间的64字节gab
    // should be 16byte aligned应该16字节对齐
    void* stack_top = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(storage) - static_cast<uintptr_t>(64));
    void* stack_bottom = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
    // create fast-context创建快速上下文
    const size_t size = reinterpret_cast<uintptr_t>(stack_top) - reinterpret_cast<uintptr_t>(stack_bottom);
    const fcontext_t fctx = make_fcontext(stack_top, size, &ctx_entry< Record >);
    assert(nullptr != fctx); // transfer control structure to context-stack将控制结构传输到上下文堆栈
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
    record(stack_context sctx, fixedsize_stack&& salloc, Fn&& fn) noexcept :
      sctx_(sctx), salloc_(std::forward< fixedsize_stack >(salloc)), fn_(std::forward< Fn >(fn)) {}
    record(record const&) = delete; record& operator=(record const&) = delete;
    void deallocate() noexcept { destroy(this); }
    fcontext_t run(fcontext_t fctx) {
      Ctx c{ fctx }; c = std::invoke(fn_, std::move(c)); return std::exchange(c.fctx_, nullptr);
    }
  };
  class co {
  private:
    template<typename Ctx, typename Fn> friend class record;
    template<typename Ctx, typename Fn> friend transfer_t ctx_ontop(transfer_t);
    fcontext_t  fctx_{ nullptr };
    template<typename Fn>
    friend co callcc(std::allocator_arg_t, fixedsize_stack&&, Fn&&);
    co resume()&& {
      assert(nullptr != fctx_); return { jump_fcontext(std::exchange(fctx_, nullptr), nullptr).fctx };
    }
  public:
    co(fcontext_t fctx) noexcept : fctx_{ fctx } {}
    co() noexcept = default;

    //template<typename Fn, typename = std::disable_overload< co, Fn >>
    //co(fixedsize_stack& salloc): co{ std::allocator_arg, salloc } {}
    template<typename Fn, typename = std::disable_overload< co, Fn >>
    co(Fn&& fn) : co{ std::allocator_arg, fixedsize_stack(65536), std::forward< Fn >(fn) } {}
    template<typename Fn>
    co(std::allocator_arg_t, fixedsize_stack&& salloc, Fn&& fn) :
      fctx_{ add_ctx< record< co, Fn > >(std::forward< fixedsize_stack >(salloc), std::forward< Fn >(fn)) } {}
    //fiber
    ~co() {
      if (_unlikely(nullptr != fctx_)) {
        ontop_fcontext(std::exchange(fctx_, nullptr), nullptr, ctx_unwind);
      }
    }
    co(co&& other) noexcept { std::swap(fctx_, other.fctx_); }
    co& operator=(co&& other) noexcept {
      if (_likely(this != &other)) { std::swap(fctx_, other.fctx_); } return *this;
    }
    co(co const& other) noexcept = delete;
    co& operator=(co const& other) noexcept = delete;
    _FORCE_INLINE co yield()& { return std::move(*this).resume(); }
    template< typename Fn >
    co resume_with(Fn&& fn)& { return std::move(*this).resume_with(std::forward< Fn >(fn)); }
    template< typename Fn >
    co resume_with(Fn&& fn)&& {
      assert(nullptr != fctx_); auto p = std::make_tuple(std::forward< Fn >(fn));
      return { ontop_fcontext(std::exchange(fctx_, nullptr), &p, ctx_ontop< co, Fn >).fctx };
    }
    explicit operator bool() const noexcept { return nullptr != fctx_; }
    bool operator!() const noexcept { return nullptr == fctx_; }
    bool operator<(co const& other) const noexcept { return fctx_ < other.fctx_; }
    template< typename charT, class traitsT >
    friend std::basic_ostream< charT, traitsT >&
      operator<<(std::basic_ostream< charT, traitsT >& os, co const& other) {
      if (nullptr != other.fctx_) { return os << other.fctx_; }
      else { return os << "{not-a-context}"; }
    }
  };
  template<typename Fn>
  co callcc(Fn&& fn, fixedsize_stack&& s = fixedsize_stack(65536)) {
    return callcc(std::allocator_arg, std::forward<fixedsize_stack>(s), std::forward<Fn>(fn));
  };
  template<typename Fn>
  co callcc(std::allocator_arg_t, fixedsize_stack&& salloc, Fn&& fn) {
    return co{ add_ctx<record<co, Fn>>(std::forward<fixedsize_stack>(salloc), std::forward< Fn >(fn)) }.resume();
  }
  typedef co fiber;
}
namespace fc {
  typedef ctx::co co;
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