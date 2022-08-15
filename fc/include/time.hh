#ifndef TIME_HH
#define TIME_HH
#include <chrono>
#include <thread>
//from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/timer.hh
//from https://github.com/idealvin/coost/blob/master/include/co/time.h
/* using nanoseconds = duration<long long, nano>;
  using microseconds = duration<long long, micro>;
  using milliseconds = duration<long long, milli>;
  using seconds      = duration<long long>;
  using minutes      = duration<int, ratio<60>>;
  using hours        = duration<int, ratio<3600>>;
  */
namespace fc {
  namespace sleep {
	void h(long long h);
	void m(long long m);
	void s(long long s);
	void ms(long long ms);
	void us(long long us);
	void ns(long long ns);
  }
  class Time {
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
  public:
	Time();
	void restart();
	long long ms() const;
	long long us() const;
	long long ns() const;
  };
}
#endif
