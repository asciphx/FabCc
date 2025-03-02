#ifndef CTX_HH
#define CTX_HH
/*
 * This software is licensed under the AGPL-3.0 License.
 *
 * Copyright (C) 2025 Asciphx
 *
 * Permissions of this strongest copyleft license are conditioned on making available
 * complete source code of licensed works and modifications, which include larger works
 * using a licensed work, under the same license. Copyright and license notices must be
 * preserved. Contributors provide an express grant of patent rights. When a modified
 * version is used to provide a service over a network, the complete source code of
 * the modified version must be made available.
 */
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
#include "hh/timer.hh"
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
#if _OPENSSL
#define CTX_MIN_SIZE stack_traits::default_size()
#else
#define CTX_MIN_SIZE 65536
#endif
namespace fc {
  struct ROG;
}
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
    //Black magic, using box<> to automatically manage the release of objects
    std::unique_ptr<fc::ROG> box;
    //template<typename Fn, typename = std::disable_overload< co, Fn >>
    //co(fixedsize_stack& salloc): co{ std::allocator_arg, salloc } {}
    template<typename Fn, typename = std::disable_overload< co, Fn >>
    co(Fn&& fn) : co{ std::allocator_arg, fixedsize_stack(CTX_MIN_SIZE), std::forward< Fn >(fn) } {}
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
    _FORCE_INLINE void operator()() { *this = std::move(*this).resume(); }
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
  co callcc(Fn&& fn, fixedsize_stack&& s = fixedsize_stack(CTX_MIN_SIZE)) {
    return callcc(std::allocator_arg, std::forward<fixedsize_stack>(s), std::forward<Fn>(fn));
  };
  template<typename Fn>
  co callcc(std::allocator_arg_t, fixedsize_stack&& salloc, Fn&& fn) {
    return co{ add_ctx<record<co, Fn>>(std::forward<fixedsize_stack>(salloc), std::forward< Fn >(fn)) }.resume();
  }
  typedef co fiber;
}
#if __cplusplus >= _cpp20_date
/*
* This software is licensed under the AGPL-3.0 License.
*
* Copyright (C) 2025 Asciphx
*
* Permissions of this strongest copyleft license are conditioned on making available
* complete source code of licensed works and modifications, which include larger works
* using a licensed work, under the same license. Copyright and license notices must be
* preserved. Contributors provide an express grant of patent rights. When a modified
* version is used to provide a service over a network, the complete source code of
* the modified version must be made available.
*/// C++20 Coroutine for task - Asciphx
#include <coroutine>
namespace fc {
  template <typename T = void>
  struct Task {
    Task() noexcept = default; Task(Task const&) = delete;
    struct promise_type; Task& operator=(Task const&) = delete;
    Task& operator=(Task&& _) noexcept { $ = std::exchange(_.$, nullptr); return *this; }
    using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
    Task(promise_type* p): $(Handle::from_promise(*p)) {}
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
    ~Task() { if ($) $.destroy(); }
    struct promise_type {
      std::coroutine_handle<> l, r; mutable T _{}; std::exception_ptr e;
      void unhandled_exception() { e = std::current_exception(); }
      std::suspend_never initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      std::suspend_always yield_value(T&& v) { _ = std::move(v); return {}; }
      std::suspend_always yield_value(const T& v) { _ = v; return {}; }
      _FORCE_INLINE Task get_return_object() { return Task{ this }; }
      _FORCE_INLINE void return_value(const T& v) { _ = v; }
      _FORCE_INLINE void return_value(T&& v) { _ = T(std::move(v)); }
    };
    _FORCE_INLINE bool await_ready() const noexcept { return $.done(); }
    _FORCE_INLINE T await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); return std::move($.promise()._); }
    _FORCE_INLINE void await_suspend(Handle _) noexcept { _.promise().l = $; $.promise().r = _; }
    template<typename S> void await_suspend(std::coroutine_handle<S> _) noexcept { _.promise().l = $; $.promise().r = _; }
    explicit operator bool() const noexcept { return $ && !$.done(); }
    void operator()() noexcept {
      while ($.promise().l) { if (($ = Handle::from_address($.promise().l.address()))) continue; break; };
      do {
        $.resume(); if (!$.done())return;
        if ($.promise().r) $ = Handle::from_address($.promise().r.address()), $.promise().l = nullptr;
      } while (!$.done());
    }
    _FORCE_INLINE T get() { while ($.promise().l && ($ = Handle::from_address($.promise().l.address()))){}; return $.promise()._; }
  };
  template <> struct Task<void> {
    Task() noexcept = default; Task(Task const&) = delete;
    struct promise_type; Task& operator=(Task const&) = delete;
    Task& operator=(Task&& _) noexcept { $ = std::exchange(_.$, nullptr); return *this; }
    using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
    Task(promise_type* p): $(Handle::from_promise(*p)) {}
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
    ~Task() { if ($) $.destroy(); }
    struct promise_type {
      std::coroutine_handle<> l, r; std::exception_ptr e; void return_void() {}
      _FORCE_INLINE void unhandled_exception() { e = std::current_exception(); }
      std::suspend_never initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      _FORCE_INLINE Task get_return_object() { return Task{ this }; }
    };
    _FORCE_INLINE bool await_ready() const noexcept { return $.done(); }
    _FORCE_INLINE void await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); }
    _FORCE_INLINE void await_suspend(Handle _) noexcept { _.promise().l = $; $.promise().r = _; }
    template<typename T> void await_suspend(std::coroutine_handle<T> _) noexcept { _.promise().l = $; $.promise().r = _; }
    explicit operator bool() const noexcept { return $ && !$.done(); }
    void operator()() noexcept {
      while ($.promise().l) { if (($ = Handle::from_address($.promise().l.address()))) continue; break; };
      do {
        $.resume(); if (!$.done())return;
        if ($.promise().r) $ = Handle::from_address($.promise().r.address()), $.promise().l = nullptr;
      } while (!$.done());
    }
  };
  template <> struct Task<ROG> {
    Task() noexcept = default; Task(Task const&) = delete;
    struct promise_type; Task& operator=(Task const&) = delete;
    Task& operator=(Task&& _) noexcept { $ = std::exchange(_.$, nullptr); return *this; }
    Task& operator=(Task<void>&& _) noexcept { $ = Handle::from_address(_.$.address()); _.$ = nullptr; return *this; }
    using Handle = std::coroutine_handle<promise_type>; mutable Handle $; std::unique_ptr<ROG> box;//Black magic
    Task(promise_type* p): $(Handle::from_promise(*p)) {}
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
    ~Task() { if ($) $.destroy(); }
    struct promise_type {
      std::coroutine_handle<> l, r; std::exception_ptr e; void return_void() {}
      _FORCE_INLINE void unhandled_exception() { e = std::current_exception(); }
      std::suspend_never initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      _FORCE_INLINE Task get_return_object() { return Task{ this }; }
    };
    _FORCE_INLINE bool await_ready() const noexcept { return $.done(); }
    _FORCE_INLINE void await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); }
    _FORCE_INLINE void await_suspend(Handle _) noexcept { _.promise().l = $; $.promise().r = _; }
    template<typename T> void await_suspend(std::coroutine_handle<T> _) noexcept { _.promise().l = $; $.promise().r = _; }
    explicit operator bool() const noexcept { return $ && !$.done(); }
    void operator()() noexcept {
      while ($.promise().l) { if (($ = Handle::from_address($.promise().l.address()))) continue; break; };
      do {
        $.resume(); if (!$.done())return;
        if ($.promise().r) $ = Handle::from_address($.promise().r.address()), $.promise().l = nullptr;
      } while (!$.done());
    }
  };
}
#define _CTX_TASK(_) fc::Task<_>
#define _CTX_back(_) co_return _;
#define _CTX_return co_return;
#define _ctx -> fc::Task<>
#else
#define _CTX_TASK(_) _
#define _CTX_back(_) return _;
#define _CTX_return return;
#define _ctx -> void
#define co_return
#define co_await
#endif
namespace fc {
  typedef ctx::co co;
#if defined _WIN32
  typedef unsigned long long socket_type;//SD_RECEIVE，SD_SEND，SD_BOTH
#else
  typedef int socket_type;
#endif
  //This is a cross platform coroutine wrapper, designed for convenience and practicality
  struct ROG {
    Timer::Node t_id;
#if __linux__ || _WIN32
    socket_type $;
    ROG(socket_type s):$(s) {}
#else
    ROG(socket_type s) : {}
#endif
    int64_t hrt{ 0 };
#if __cplusplus < _cpp20_date
    ctx::co _;
#else
    fc::Task<ROG> _;
#endif
    ROG():$(0) {}
  };
}
#undef CTX_MIN_SIZE
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