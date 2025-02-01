#ifndef CPP_HH
#define CPP_HH
#include <tuple>
#include <functional>
#include <type_traits>
#include <utility>
#include <time.h>
#include <exception>
#if (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
#define _unlikely(x) __builtin_expect((x),0)
#define _likely(x) __builtin_expect(!!(x),1)
#else
#define _unlikely(x) x
#define _likely(x) x
#endif
#define _PTR_LEN sizeof(void*)
typedef float f32;
typedef double f64;
typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
#ifdef _MSVC_LANG
#define _FORCE_INLINE __forceinline
#define _ALIGN(s) __declspec(align(s))
#define _NEVER_INLINE __declspec(noinline)
#define __thread __declspec(thread)
#define _cpp20_date 201705L
#else
#define _ALIGN(s) __attribute__((aligned(s)))
#define _FORCE_INLINE inline __attribute__((always_inline))
#define _NEVER_INLINE inline __attribute__((noinline))
#define _cpp20_date 201709L
#endif
#if __cplusplus >= _cpp20_date
#include <coroutine>
/*
* This software is licensed under the AGPL-3.0 License.
*
* Copyright (C) 2023 Asciphx
*
* Permissions of this strongest copyleft license are conditioned on making available
* complete source code of licensed works and modifications, which include larger works
* using a licensed work, under the same license. Copyright and license notices must be
* preserved. Contributors provide an express grant of patent rights. When a modified
* version is used to provide a service over a network, the complete source code of
* the modified version must be made available.
*/// C++20 Coroutine for task - Asciphx
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
#ifdef _MSVC_LANG
#ifdef _WIN64
typedef signed __int64 ssize_t;
#elif _WIN32
typedef signed int ssize_t;
#endif
#endif // _MSVC_LANG
namespace std {
#if (defined(_HAS_CXX17) && _HAS_CXX17 == 1) || (defined(__cplusplus) && __cplusplus >= 201703L)
#define __INLINE inline
#define __CONSTEXPR constexpr
#define __CONSTEXPR14 constexpr
#elif defined(__cplusplus) && __cplusplus >= 201402L
#define __INLINE
#define __CONSTEXPR
#define __CONSTEXPR14 constexpr
  template< typename Fn, typename ... Args >
  typename std::enable_if<
    std::is_member_pointer< typename std::decay< Fn >::type >::value,
    typename std::result_of< Fn && (Args && ...) >::type
  >::type
    invoke(Fn&& fn, Args && ... args) {
    return std::mem_fn(fn)(std::forward< Args >(args) ...);
  }
  template< typename Fn, typename ... Args >
  typename std::enable_if<
    !std::is_member_pointer< typename std::decay< Fn >::type >::value,
    typename std::result_of< Fn && (Args && ...) >::type
  >::type
    invoke(Fn&& fn, Args && ... args) {
    return std::forward< Fn >(fn)(std::forward< Args >(args) ...);
  }
  struct in_place_t { // tag used to select a constructor which initializes a contained object in place
    explicit in_place_t() = default;
  };
  constexpr in_place_t in_place{};
  template <class>
  struct in_place_type_t { // tag that selects a type to construct in place
    explicit in_place_type_t() = default;
  };
  template <class _Ty>
  constexpr in_place_type_t<_Ty> in_place_type{};
  /// Used to represent an optional with no data; essentially a bool
  class monostate {};
#else
#define __INLINE
#define __CONSTEXPR
#define __CONSTEXPR14 static inline
  template< typename Fn, typename ... Args >
  typename std::enable_if<
    std::is_member_pointer< typename std::decay< Fn >::type >::value,
    typename std::result_of< Fn && (Args && ...) >::type
  >::type
    invoke(Fn&& fn, Args && ... args) {
    return std::mem_fn(fn)(std::forward< Args >(args) ...);
  }
  template< typename Fn, typename ... Args >
  typename std::enable_if<
    !std::is_member_pointer< typename std::decay< Fn >::type >::value,
    typename std::result_of< Fn && (Args && ...) >::type
  >::type
    invoke(Fn&& fn, Args && ... args) {
    return std::forward< Fn >(fn)(std::forward< Args >(args) ...);
  }
  struct in_place_t { // tag used to select a constructor which initializes a contained object in place
    explicit in_place_t() = default;
  };
  template <class>
  struct in_place_type_t { // tag that selects a type to construct in place
    explicit in_place_type_t() = default;
  };
  template <class _Ty>
  struct in_place_type {
    explicit in_place_type() = default;
  };
  template <class T>
  using remove_const_t = typename std::remove_const<T>::type;
  template <class T>
  using remove_reference_t = typename std::remove_reference<T>::type;
  template <class T>
  using decay_t = typename std::decay<T>::type;
  template <bool E, class T = void>
  using enable_if_t = typename std::enable_if<E, T>::type;
  template <bool B, class T, class F>
  using conditional_t = typename std::conditional<B, T, F>::type;
#endif
  // http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
  template< typename X, typename Y >
  using disable_overload =
    typename std::enable_if<
    !std::is_base_of<
    X,
    typename std::decay< Y >::type
    >::value
    >::type;
#ifndef _MSVC_LANG
#if (defined(__cplusplus) && __cplusplus < 201402L)
  template <typename T, typename U = T> inline T exchange(T& _, U&& $) { return std::__exchange(_, std::forward<U>($)); }
#endif
#endif
  template <class> struct Tuple {};
}
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>//multimap
#include <stack>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#endif // !CPP_HH