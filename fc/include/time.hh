#ifndef TIME_HH
#define TIME_HH
#include <chrono>
#include <thread>
/* using nanoseconds = duration<long long, nano>;
  using microseconds = duration<long long, micro>;
  using milliseconds = duration<long long, milli>;
  using seconds      = duration<long long>;
  using minutes      = duration<int, ratio<60>>;
  using hours        = duration<int, ratio<3600>>; */
namespace fc {
  namespace sleep {
    void h(int h);
    void m(int m);
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
    int h() const;
    int m() const;
    long long s() const;
    long long ms() const;
    long long us() const;
    long long ns() const;
  };
}
#endif
