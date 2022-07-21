#ifndef TIMER_HH
#define TIMER_HH
#include <uv.h>
#include <functional>
#include <h/common.h>
namespace fc {
  static struct tm* RES_NOW;
  static int64_t RES_TIME_T, RES_last;
  static std::string RES_DATE_STR;
  tm now();
  int64_t nowStamp(short& i);
  int64_t nowStamp(short&& i);
  int64_t nowStamp();
  class Timer {
	uv_timer_t* t_;
	std::function<void()> cb_;
	bool alive{ true };
	static void timer_cb(uv_timer_t* handle);
  public:
	Timer(uv_loop_t* loop = uv_default_loop());
	~Timer();
	void setTimeout(std::function<void()>&& func, uint32_t milliseconds);
	void stop();
  };
}
#endif
