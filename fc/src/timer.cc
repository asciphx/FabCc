#include "timer.hh"
namespace fc {
  void Timer::setTimeout(std::function<void()>&& func, uint32_t milliseconds) {
	alive = true; std::thread t([=]() {
	  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	  if (!alive.load()) return; func();
	  });
	t.detach();
  };
  void Timer::setInterval(std::function<void()>&& func, uint32_t milliseconds) {
	alive = true; std::thread t([=]() {
	  while (alive.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
		if (!alive.load()) return; func();
	  }
	  });
	t.detach();
  };
  void Timer::setTimeoutSec(std::function<void()>&& func, uint32_t seconds) {
	alive = true; std::thread t([=]() {
	  std::this_thread::sleep_for(std::chrono::seconds(seconds));
	  if (!alive.load()) return; func();
	  });
	t.detach();
  };
  void Timer::setIntervalSec(std::function<void()>&& func, uint32_t seconds) {
	alive = true; std::thread t([=]() {
	  while (alive.load()) {
		std::this_thread::sleep_for(std::chrono::seconds(seconds));
		if (!alive.load()) return; func();
	  }
	  });
	t.detach();
  };
  void Timer::stop() { alive = false; }
}