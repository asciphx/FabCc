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
template <typename T = void>
struct Task {
  Task() noexcept = default; Task(Task const&) noexcept = delete;
  struct promise_type; Task& operator=(Task const&) noexcept = delete;
  Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
  Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
  using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
  Task(promise_type* p): $(Handle::from_promise(*p)) {}
  struct promise_type {
    std::coroutine_handle<> l, r; T _; std::exception_ptr e;
    void unhandled_exception() { e = std::current_exception(); }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    std::suspend_always yield_value(T&& v) { _ = std::move(v); return {}; }
    std::suspend_always yield_value(T& v) { _ = v; return {}; }
    Task get_return_object() { return Task{ this }; }
    void return_value(const T& v) { _ = v; }
    void return_value(T&& v) { _ = T(std::move(v)); }
  };
  bool await_ready() { return !$ || $.done(); }
  T await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); return std::move($.promise()._); }
  void await_suspend(Handle _) { _.promise().l = $, $.promise().r = _; }
  explicit operator bool() { return $ && !$.done(); }
  template<typename U>
  void await_suspend(std::coroutine_handle<U> _) {
    _.promise().l = $, $.promise().r = _;
  }
  void operator()() {
    while ($) {
      if (!$.promise().l) {
        while (!$.done()) {
          $.resume(); if (!$.done())break;
          if ($.promise().r) $ = Handle::from_address($.promise().r.address()), $.promise().l = nullptr;
        } return;
      }
      $ = Handle::from_address($.promise().l.address());
    }
  }
  ~Task() { if ($) $.destroy(); }
  T get() { while ($ && $.promise().l && ($ = Handle::from_address($.promise().l.address()))) {}; return $.promise()._; }
};
template <> struct Task<void> {
  Task() noexcept = default; Task(Task const&) noexcept = delete;
  struct promise_type; Task& operator=(Task const&) noexcept = delete;
  Task& operator=(Task&& _) noexcept { $ = _.$; _.$ = nullptr; return *this; }
  Task(Task&& t) noexcept: $(t.$) { t.$ = nullptr; }
  using Handle = std::coroutine_handle<promise_type>; mutable Handle $;
  Task(promise_type* p): $(Handle::from_promise(*p)) {}
  struct promise_type {
    std::coroutine_handle<> l, r; std::exception_ptr e; void return_void() {}
    void unhandled_exception() { e = std::current_exception(); }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    Task get_return_object() { return Task{ this }; }
  };
  bool await_ready() { return !$ || $.done(); }
  void await_resume() { if ($.promise().e) std::rethrow_exception($.promise().e); }
  void await_suspend(Handle _) { _.promise().l = $, $.promise().r = _; }
  explicit operator bool() { return $ && !$.done(); } ~Task() { if ($) $.destroy(); }
  template<typename T>
  void await_suspend(std::coroutine_handle<T> _) {
    _.promise().l = $, $.promise().r = _;
  }
  void operator()() {
    while ($) {
      if (!$.promise().l) {
        while (!$.done()) {
          $.resume(); if (!$.done())break;
          if ($.promise().r) $ = Handle::from_address($.promise().r.address()), $.promise().l = nullptr;
        } return;
      }
      $ = Handle::from_address($.promise().l.address());
    }
  }
};
Task<> coro_0() {
  std::cout << '4'; co_await std::suspend_always{}; std::cout << '5'; co_return;
}
Task<int> coro_1() {
  std::cout << '3';
  co_await coro_0(); std::cout << '6'; co_return 3;
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
int main() {
  Task<int> task = all();
  int yield_times = 0;
  while (task) { ++yield_times; std::cout << '\'' << task.get() << '\''; task(); }
  std::cout << " => " << yield_times << ", " << task.get() << '\n';
  //1234'0'5678234'0'567 => 2, 16
  return 0;
}