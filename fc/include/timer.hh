#ifndef TIMER_HH
#define TIMER_HH
#include <uv.h>
#include <functional>
namespace fc {
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
