#ifndef TIMER_HH
#define TIMER_HH
#include <uv.h>
#include <functional>
#include <string>
namespace fc {
  static struct tm* RES_NOW;
  static int64_t RES_TIME_T, RES_last;
  static std::string RES_DATE_STR;
  static inline tm now() { return *RES_NOW; }
  static inline int64_t nowStamp(short& i) { return RES_TIME_T + i; }
  static inline int64_t nowStamp(short&& i) { return RES_TIME_T + i; }
  static inline int64_t nowStamp() { return RES_TIME_T; }
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
