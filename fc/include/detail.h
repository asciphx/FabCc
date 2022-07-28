#pragma once
#include <unordered_map>
#include <functional>
#include <chrono>
#include <thread>
#include <fstream>
#include <iterator>
#include <map>
namespace fc {
  class dumb_timer_queue {
  public:
	static unsigned short tick;//
	// dumb_timer_queue(boost::asio::io_service& io_service) :
	   //io_service_(io_service), deadline_timer_(io_service_) {
	   //deadline_timer_.expires_from_now(boost::posix_time::seconds(1));
	   //deadline_timer_.async_wait(std::bind(&dumb_timer_queue::tick_handler, this, std::placeholders::_1));
	// }
	~dumb_timer_queue() { /*deadline_timer_.cancel();*/ }
	inline void cancel(uint16_t& id) {
	  LOG_GER(" -key:< " << id << " >"); dq_.erase(id); id = 0;
	}
	inline size_t add(const std::function<void()>& task) {
	  if (step_ == 0xffff)++step_; dq_.insert({ ++step_, {std::chrono::steady_clock::now() + std::chrono::seconds(tick), task} }); return step_;
	}
	inline size_t add(const std::function<void()>& task, std::uint8_t& timeout) {
	  if (step_ == 0xffff)++step_; dq_.insert({ ++step_, {std::chrono::steady_clock::now() + std::chrono::seconds(timeout), task} }); return step_;
	}
	inline size_t add(const std::function<void()>& task, std::uint8_t&& timeout) {
	  if (step_ == 0xffff)++step_; dq_.insert({ ++step_, {std::chrono::steady_clock::now() + std::chrono::seconds(timeout), task} }); return step_;
	}
  private:
	void tick_handler(.../*const boost::system::error_code& ec*/) {
	  //if (ec) return;
#ifdef _WIN32
	  for (const auto& task : dq_) {
		if (task.second.first < std::chrono::steady_clock::now()) {
		  (task.second.second)(); dq_.erase(task.first);
		}
	  }
#else
	  std::vector<uint16_t> vts; for (const auto& task : dq_) {
		if (task.second.first < std::chrono::steady_clock::now()) { (task.second.second)(); vts.push_back(task.first); }
	  } for (const auto& task : vts) dq_.erase(task);
#endif
	  //deadline_timer_.expires_from_now(boost::posix_time::seconds(1));
	  //deadline_timer_.async_wait(std::bind(&dumb_timer_queue::tick_handler, this, std::placeholders::_1));
	}
  private:
	std::map<uint16_t, std::pair<std::chrono::steady_clock::time_point, std::function<void()>>> dq_;
	uint16_t step_{ 0 };
	//boost::asio::io_service& io_service_;
	//boost::asio::deadline_timer deadline_timer_;
  };
}
