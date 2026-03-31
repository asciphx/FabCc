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
#include "fcontext.h"
#include <cstdlib>
#include <new>
#include <assert.h>
#include <ostream>
#include <exception>
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
#define CTX_MIN_SIZE 131072
#else
#define CTX_MIN_SIZE 65536
#endif
namespace fc {
  struct ROG;
}
namespace ctx {
  class co; struct FN { co (*func)(void*, co&&); void* ctx; void (*destroy)(void*); };
  static constexpr uintptr_t stack_align_mask = (_PTR_LEN == 8) ? 0X7f : 0X3f;
  static constexpr uintptr_t stack_gap = (_PTR_LEN == 8) ? 0X40 : 0X20;
  static const constexpr size_t fixedsize = CTX_MIN_SIZE;
  struct forced_unwind {
    fcontext_t fctx{ nullptr }; forced_unwind() = default; forced_unwind(fcontext_t fctx_): fctx(fctx_) {}
  };
  _FORCE_INLINE transfer_t ctx_unwind(transfer_t t) { throw forced_unwind(t.fctx); return { nullptr, nullptr }; }
  class record {
  private:
    void* sctx_; FN fn_;
    static void destroy(record* p) noexcept {
      void* sctx = p->sctx_; p->~record(); std::free(static_cast<char*>(sctx) - fixedsize);// destroy stack with stack allocator
    }
  public:
    record(void* sctx, FN&& fn) noexcept: sctx_(sctx), fn_(std::move(fn)) {}
    record(record const&) = delete; record& operator=(record const&) = delete;
    void deallocate() noexcept { destroy(this); } fcontext_t run(fcontext_t fctx);
  };
  _FORCE_INLINE transfer_t ctx_exit(transfer_t t) noexcept {
    record* rec = static_cast<record*>(t.data); rec->deallocate(); return { nullptr, nullptr }; // destroy context stack
  }
  _FORCE_INLINE void ctx_entry(transfer_t t) noexcept {
    // transfer control structure to the context-stack
    record* rec = static_cast<record*>(t.data);// assert(nullptr != t.fctx); assert(nullptr != rec);
    try {// jump back to `add_ctx()`
      t = jump_fcontext(t.fctx, nullptr); t.fctx = rec->run(t.fctx);// start executing
    } catch (forced_unwind const& ex) { t = { ex.fctx, nullptr }; }
    assert(nullptr != t.fctx); // destroy context-stack of `this`context on next context
    ontop_fcontext(t.fctx, rec, ctx_exit);
  }
  transfer_t ctx_ontop(transfer_t t);
  class co {
  private:
    friend class record;  friend transfer_t ctx_ontop(transfer_t); friend co callcc(FN&&);
    fcontext_t fctx_{ nullptr };
    co resume()&& { assert(nullptr != fctx_); return { jump_fcontext(std::exchange(fctx_, nullptr), nullptr).fctx }; }
  public:
    co(fcontext_t fctx) noexcept: fctx_{ fctx } {}
    co() noexcept = default;
    //Black magic, using box to automatically manage the release of objects
    std::unique_ptr<fc::ROG> box;
    // template<typename = std::disable_overload<co, FN>>
    co(FN&& fn) {
      void* sctx = static_cast<char*>(std::malloc(fixedsize)) + fixedsize;
      // reserve space for control structure控制结构预留空间
      void* storage = reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(sctx) - static_cast<uintptr_t>(sizeof(record))) & ~static_cast<uintptr_t>(stack_align_mask));
      // 64byte gab between control structure and stack top控制结构和栈顶之间的64字节gab
      void* stack_top = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(storage) - static_cast<uintptr_t>(stack_gap));
      // should be 16byte aligned应该16字节对齐
      void* stack_bottom = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(sctx) - static_cast<uintptr_t>(fixedsize));
      // placment new for control structure on context stack在上下文堆栈上放置新的控件结构
      record* rec = new(storage)record{ sctx, std::move(fn) };
      // create fast-context创建快速上下文
      const size_t size = reinterpret_cast<uintptr_t>(stack_top) - reinterpret_cast<uintptr_t>(stack_bottom);
      const fcontext_t fctx = make_fcontext(stack_top, size, &ctx_entry); assert(nullptr != fctx);
      // transfer control structure to context-stack将控制结构传输到上下文堆栈
      fctx_ = jump_fcontext(fctx, rec).fctx;
    }
    ~co() {
      if (_unlikely(nullptr != fctx_)) ontop_fcontext(std::exchange(fctx_, nullptr), nullptr, ctx_unwind);
    }
    co(co&& other) noexcept { std::swap(fctx_, other.fctx_); }
    _FORCE_INLINE co& operator=(co&& other) noexcept {
      if (_likely(this != &other)) { std::swap(fctx_, other.fctx_); } return *this;
    }
    co(co const& other) noexcept = delete;
    co& operator=(co const& other) noexcept = delete;
    _FORCE_INLINE co yield()& { return std::move(*this).resume(); }
    _FORCE_INLINE void operator()() { *this = std::move(*this).resume(); }
    co resume_with(FN&& fn)& { return std::move(*this).resume_with(std::move(fn)); }
    co resume_with(FN fn)&& {
      assert(nullptr != fctx_); return { ontop_fcontext(std::exchange(fctx_, nullptr), &fn, ctx_ontop).fctx };
    }
    explicit operator bool() const noexcept { return nullptr != fctx_; }
    bool operator!() const noexcept { return nullptr == fctx_; }
    bool operator<(co const& other) const noexcept { return fctx_ < other.fctx_; }
  };
  _FORCE_INLINE co callcc(FN&& f) { return co{ std::move(f) }.resume(); };
  _FORCE_INLINE transfer_t ctx_ontop(transfer_t t) {
    assert(nullptr != t.data); FN* fn = static_cast<FN*>(t.data); t.data = nullptr;
    co c{ t.fctx };// execute function, pass continuation via reference
    c = fn->func(fn->ctx, std::move(c)); fn->destroy(fn->ctx); return { std::exchange(c.fctx_, nullptr), nullptr };
  }
  _FORCE_INLINE fcontext_t record::run(fcontext_t fctx) {//c = std::invoke(fn_.func, fn_.ctx, std::move(c));
    co c{ fctx }; c = fn_.func(fn_.ctx, std::move(c)); return std::exchange(c.fctx_, nullptr);
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
      void* l, * r; std::exception_ptr e; void return_void() {}
      _FORCE_INLINE void unhandled_exception() { e = std::current_exception(); }
      std::suspend_never initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      _FORCE_INLINE Task get_return_object() { return Task{ this }; }
    };
    _FORCE_INLINE bool await_ready() const noexcept { return $.done(); }
    _FORCE_INLINE void await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); }
    _FORCE_INLINE void await_suspend(Handle _) noexcept { _.promise().l = $.address(); $.promise().r = _.address(); }
    template<typename T> void await_suspend(std::coroutine_handle<T> _) noexcept { _.promise().l = $; $.promise().r = _; }
    explicit operator bool() const noexcept { return $ && !$.done(); }
    void operator()() noexcept {
      while ($.promise().l) { if (($ = Handle::from_address($.promise().l))) continue; break; };
      do {
        $.resume(); if (!$.done())return;
        if ($.promise().r) $ = Handle::from_address($.promise().r), $.promise().l = nullptr;
      } while (!$.done());
    }
  };
}
#define _CTX_TASK(_) fc::Task<_>
#define _CTX_back(_) co_return _;
#define _CTX_return co_return
#define _ctx -> fc::Task<>
#define CTX_LAMBDA(...)
#else
#define CTX_LAMBDA(name, CAPTURE_DEF)         \
  struct name##_lambda CAPTURE_DEF; static ctx::co name##_func(void* ctx, ctx::co&& sink)
#define CTX_LAMBDA_IMPL(name)                 \
  static ctx::co name##_func(void* ctx, ctx::co&& sink)
#define CTX_CALLCC(name, ...) name##_func, new name##_lambda{__VA_ARGS__} , [](void* x){ delete static_cast<name##_lambda*>(x); }
#define _CTX_TASK(_) _
#define _CTX_back(_) return _;
#define _CTX_return return
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
    alignas(16) int64_t hrt;
    alignas(16) socket_type ${ 0 };
    ROG(socket_type s):$(s) {}
#endif
#if __cplusplus < _cpp20_date
    ctx::co _;
#else
    fc::Task<ROG> _;
#endif
#if __APPLE__
    int64_t hrt;
#endif
    ROG():hrt(0) {}
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