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
// C++20 Coroutines - Asciphx
namespace {
  template <typename T>
  struct Task {
    struct promise_type;
    Task() noexcept = default; Task(Task const& other) noexcept = delete; Task& operator=(Task const& other) noexcept = delete;
    Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
    Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; } using Handle = std::coroutine_handle<promise_type>;
    Task(promise_type* p): $(Handle::from_promise(*p)) {} mutable Handle $;
    struct promise_type {
      T _; Handle l, r; T value() { return _; } std::suspend_never initial_suspend() { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; } void return_value(T v) { _ = v; }
      Task get_return_object() { return Task{ this }; } void unhandled_exception() { std::terminate(); }
    };
    bool await_ready() { return !$ || $.done(); } void await_suspend(Handle _) { _.promise().l = $, $.promise().r = _; }
    T await_resume() { return $.promise()._; } explicit operator bool() { return $ && !$.done(); } void await_suspend(std::coroutine_handle<> _) {}
    void operator()() {
      if (!$.promise().l) {
        while (!$.done()) { $.resume(); if (!$.done())break; if ($.promise().r) $ = $.promise().r, $.promise().l = nullptr; } return;
      }
      $ = $.promise().l;
    }
    ~Task() { if ($) $.destroy(); } T get() { return std::move($.promise()._); }
  };
  template <> struct Task<void> {
    Task() noexcept = default; Task(Task const& other) noexcept = delete; Task& operator=(Task const& other) noexcept = delete;
    Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
    struct promise_type; using Handle = std::coroutine_handle<promise_type>; Task(promise_type* p): $(Handle::from_promise(*p)) {} mutable Handle $;
    struct promise_type {
      Handle l, r; std::suspend_never initial_suspend() { return {}; } void unhandled_exception() { std::terminate(); }
      std::suspend_always final_suspend() noexcept { return {}; } Task get_return_object() { return Task{ this }; } void return_void() {}
    };
    bool await_ready() { return !$ || $.done(); } void await_suspend(Handle _) { _.promise().l = $, $.promise().r = _; } void await_resume() {}
    explicit operator bool() { return $ && !$.done(); } ~Task() { if ($) $.destroy(); } void await_suspend(std::coroutine_handle<> _) {}
    void operator()() {
      if (!$.promise().l) {
        while (!$.done()) { $.resume(); if (!$.done())break; if ($.promise().r) $ = $.promise().r, $.promise().l = nullptr; } return;
      }
      $ = $.promise().l;
    }
  };
  Task<int> coro_() {
    co_return -1;
  }
  Task<int> coro_0() {
    int i = co_await coro_(); co_return ++i;
  }
  Task<int> coro_1() {
    int i = co_await coro_0(); co_return ++i;
  }
  Task<int> coro_2() {
    int i = co_await coro_1(); co_return ++i;
  }
  Task<int> all() {
    co_return co_await coro_1() + co_await coro_2();
  }
}
int main() {
  Task<int> task = all();
  while (task) task();
  std::cout << task.get() << '\n';
  //3
  return 0;
}