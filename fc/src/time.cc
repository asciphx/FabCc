#include "time.hh"
//from https://github.com/idealvin/coost/blob/master/src/time.cc
//from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/timer.hh
namespace fc {
  void sleep::h(int h) { std::this_thread::sleep_for(std::chrono::hours(h)); };
  void sleep::m(int m) { std::this_thread::sleep_for(std::chrono::minutes(m)); };
  void sleep::s(long long s) { std::this_thread::sleep_for(std::chrono::seconds(s)); };
  void sleep::ms(long long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); };
  void sleep::us(long long us) { std::this_thread::sleep_for(std::chrono::microseconds(us)); };
  void sleep::ns(long long ns) { std::this_thread::sleep_for(std::chrono::nanoseconds(ns)); };
  Time::Time() { start_ = std::chrono::high_resolution_clock::now(); }
  void Time::restart() { start_ = std::chrono::high_resolution_clock::now(); }
  int Time::h() const {
	return std::chrono::duration_cast<std::chrono::hours>(std::chrono::high_resolution_clock::now() - start_).count();
  }
  int Time::m() const {
	return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::high_resolution_clock::now() - start_).count();
  }
  long long Time::s() const {
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_).count();
  }
  long long Time::ms() const {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_).count();
  }
  long long Time::us() const {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_).count();
  }
  long long Time::ns() const {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_).count();
  }
}