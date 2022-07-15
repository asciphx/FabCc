#include "timer.hh"
namespace fc {
  Timer::Timer(uv_loop_t* loop) {
	alive = false; t_ = (uv_timer_t*)calloc(sizeof(uv_timer_t), 1);
	uv_handle_set_data((uv_handle_t*)t_, this); uv_timer_init(loop, t_);
  }
  void Timer::timer_cb(uv_timer_t* h) {
	Timer* t = (Timer*)uv_handle_get_data((uv_handle_t*)h); if (t->cb_) t->cb_();
  }
  void Timer::setTimeout(std::function<void()>&& func, uint32_t milliseconds) {
	  cb_ = std::move(func), alive = uv_timer_start(t_, timer_cb, milliseconds, 0) == 0;
  };
  Timer::~Timer() {
	if (alive) uv_timer_stop(t_); if (t_ != nullptr) {
	  uv_handle_set_data((uv_handle_t*)t_, nullptr);
	  uv_close((uv_handle_t*)t_, [](uv_handle_t* h) { free(h); });
	}
  }
  void Timer::stop() {
	if (alive && uv_timer_stop(t_) == 0) alive = false;
  }
}