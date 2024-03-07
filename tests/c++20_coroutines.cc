#include <cstdio>
#include <coroutine>
#include <iostream>
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
*/
// C++20 Coroutine - Asciphx
// https://godbolt.org/z/cbP4vacTn
namespace {
  template <typename T>
  struct Task {
    Task() noexcept = default; Task(Task const& other) noexcept = delete;
    struct promise_type; Task& operator=(Task const& other) noexcept = delete;
    Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
    using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
    Task(promise_type* p): $(Handle::from_promise(*p)) {}
    struct promise_type {
      T _; Handle l, r; T value() { return _; }
      void unhandled_exception() { std::terminate(); }
      std::suspend_never initial_suspend() { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      std::suspend_always yield_value(T&& v) { _ = std::move(v); return {}; }
      std::suspend_always yield_value(T& v) { _ = v; return {}; }
      Task get_return_object() { return Task{ this }; }
      void return_value(const T& v) { _ = v; }
      void return_value(T&& v) { _ = T(std::move(v)); }
    };
    bool await_ready() { return !$ || $.done(); } T await_resume() { return $.promise()._; }
    void await_suspend(Handle _) { _.promise().l = $, $.promise().r = _; }
    explicit operator bool() { return $ && !$.done(); } void await_suspend(std::coroutine_handle<> _) {}
    void operator()() {
      while ($) {
        if (!$.promise().l) {
          while (!$.done()) { $.resume(); if (!$.done())break; if ($.promise().r) $ = $.promise().r, $.promise().l = nullptr; } return;
        }
        $ = $.promise().l;
      }
    }
    ~Task() { if ($) $.destroy(); } T get() { while ($ && $.promise().l && ($ = $.promise().l)){}; return std::move($.promise()._); }
  };
  template <> struct Task<void> {
    Task() noexcept = default; Task(Task const& other) noexcept = delete;
    struct promise_type; Task& operator=(Task const& other) noexcept = delete;
    Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
    using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
    Task(promise_type* p): $(Handle::from_promise(*p)) {}
    struct promise_type {
      Handle l, r; void return_void() {}
      void unhandled_exception() { std::terminate(); }
      std::suspend_never initial_suspend() { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      Task get_return_object() { return Task{ this }; }
    };
    bool await_ready() { return !$ || $.done(); } void await_resume() {}
    void await_suspend(Handle _) { _.promise().l = $, $.promise().r = _; }
    explicit operator bool() { return $ && !$.done(); } ~Task() { if ($) $.destroy(); }
    void await_suspend(std::coroutine_handle<> _) {}
    void operator()() {
      while ($) {
        if (!$.promise().l) {
          while (!$.done()) { $.resume(); if (!$.done())break; if ($.promise().r) $ = $.promise().r, $.promise().l = nullptr; } return;
        }
        $ = $.promise().l;
      }
    }
  };
  Task<int> coro_0() {
    std::cout << '4'; co_yield 9; std::cout << '5'; co_return 1;//1
  }
  Task<int> coro_1() {
    std::cout << '3';
    int i = co_await coro_0(); std::cout << '6'; co_return i + 2;//3
  }
  Task<int> coro_2() {
    std::cout << '2';
    int i = co_await coro_1(); std::cout << '7'; co_return i + 3;//6
  }
  Task<int> coro_3() {
    std::cout << '1';
    int i = co_await coro_2(); std::cout << '8'; co_return i + 4;//10
  }
  Task<int> all() {
    co_return co_await coro_3() + co_await coro_2();//10+6=16
  }
}
int main() {
  Task<int> task = all();
  int yield_times = 0;
  while (task) { ++yield_times; std::cout << '\'' << task.get() << '\''; task(); }
  std::cout << " => " << yield_times << ", " << task.get() << '\n';
  //1234'9'5678234'9'567 => 2, 16
  return 0;
}