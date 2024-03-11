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
#define _cpp20_date 201705L
#else
#define _cpp20_date 202002L
#endif
#if __cplusplus >= _cpp20_date
#include <coroutine>
#define _yield(x) ()
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
  template <typename T>
  struct Task {
    Task() noexcept = default; Task(Task const& other) = delete;
    struct promise_type; Task& operator=(Task const& other) = delete;
    Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
    using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
    Task(promise_type* p): $(Handle::from_promise(*p)) {}
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
    ~Task() { if ($) $.destroy(); }
    struct promise_type {
      T _; Handle l, r; std::exception_ptr e;
      void unhandled_exception() { e = std::current_exception(); }
      std::suspend_never initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      std::suspend_always yield_value(T&& v) { _ = std::move(v); return {}; }
      std::suspend_always yield_value(const T& v) { _ = v; return {}; }
      Task get_return_object() { return Task{ this }; }
      void return_value(const T& v) { _ = v; }
      void return_value(T&& v) { _ = T(std::move(v)); }
    };
    bool await_ready() const noexcept { return !$ || $.done(); }
    void await_suspend(std::coroutine_handle<> _) noexcept {}
    void await_suspend(Handle _) noexcept { _.promise().l = $, $.promise().r = _; }
    T await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); return std::move($.promise()._); }
    explicit operator bool() const noexcept { return $ && !$.done(); }
    void operator()() noexcept {
      while ($) {
        if (_unlikely(!$.promise().l)) {
          while (!$.done()) {
            $.resume(); if (!$.done())return; if ($.promise().r) $ = $.promise().r, $.promise().l = nullptr;
          } return;
        } else $ = $.promise().l;
      }
    }
    T get() { while ($.promise().l && _likely($ = $.promise().l)) {}; return std::move($.promise()._); }
  };
  template <> struct Task<void> {
    Task() noexcept = default; Task(Task const& other) = delete;
    struct promise_type; Task& operator=(Task const& other) = delete;
    Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
    using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
    Task(promise_type* p): $(Handle::from_promise(*p)) {}
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
    ~Task() { if ($) $.destroy(); }
    struct promise_type {
      Handle l, r; std::exception_ptr e; void return_void() {}
      void unhandled_exception() { e = std::current_exception(); }
      std::suspend_never initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      Task get_return_object() { return Task{ this }; }
    };
    bool await_ready() const noexcept { return !$ || $.done(); }
    void await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); }
    void await_suspend(Handle _) noexcept { _.promise().l = $, $.promise().r = _; }
    void await_suspend(std::coroutine_handle<> _) noexcept {}
    explicit operator bool() const noexcept { return $ && !$.done(); }
    int get() { return 0; }
    void operator()() noexcept {
      while ($) {
        if (_unlikely(!$.promise().l)) {
          while (!$.done()) {
            $.resume(); if (!$.done())return; if ($.promise().r) $ = $.promise().r, $.promise().l = nullptr;
          } return;
        } else $ = $.promise().l;
      }
    }
  };
}
#else
#define _yield(x)  = x->_.yield()
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
#ifdef _MSVC_LANG
#define _FORCE_INLINE __forceinline
#define _ALIGN(s) __declspec(align(s))
#define _NEVER_INLINE __declspec(noinline)
#define __thread __declspec(thread)
#else
#define _ALIGN(s) __attribute__((aligned(s)))
#define _FORCE_INLINE inline __attribute__((always_inline))
#define _NEVER_INLINE inline __attribute__((noinline))
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