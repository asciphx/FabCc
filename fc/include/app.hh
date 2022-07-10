#pragma once
#include <functional>
#include <iostream>
#include "router.hh"
#include "http_error.hh"
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
namespace fc {
  struct App {
	App() {}
	std::function<void(Req&, Res&)>& operator[](std::string_view& r) {
	  VH& h = map_[r]; h.verb = static_cast<char>(HTTP::GET); h.url = r; return h.handler;
	}
	std::function<void(Req&, Res&)>& del(std::string_view r) {
	  VH& h = map_[r]; h.verb = static_cast<char>(HTTP::DEL); h.url = r; return h.handler;
	}
	std::function<void(Req&, Res&)>& get(std::string_view r = "") {
	  VH& h = map_[r]; h.verb = static_cast<char>(HTTP::GET); h.url = r; return h.handler;
	}
	std::function<void(Req&, Res&)>& post(std::string_view r) {
	  VH& h = map_[r]; h.verb = static_cast<char>(HTTP::POST); h.url = r; return h.handler;
	}
	std::function<void(Req&, Res&)>& put(std::string_view r) {
	  VH& h = map_[r]; h.verb = static_cast<char>(HTTP::PUT); h.url = r; return h.handler;
	}
	std::function<void(Req&, Res&)>& patch(std::string_view r) {
	  VH& h = map_[r]; h.verb = static_cast<char>(HTTP::PATCH); h.url = r; return h.handler;
	}
	char sv2c(std::string_view method) const {
	  switch (hack8Str(method.data())) {
	  case "DELETE"_l:return static_cast<char>(HTTP::DEL);
	  case 4670804:return static_cast<char>(HTTP::GET);
	  case 1347375956:return static_cast<char>(HTTP::POST);
	  case 5264724:return static_cast<char>(HTTP::PUT);
	  case "PATCH"_l:return static_cast<char>(HTTP::PATCH);
		//case 1212498244:return static_cast<char>(HTTP::HEAD);
		//case "CONNECT"_l:return static_cast<char>(HTTP::CONNECT);
		//case "OPTIONS"_l:return static_cast<char>(HTTP::OPTIONS);
		//case "TRACE"_l:return static_cast<char>(HTTP::TRACE);
		//case "PURGE"_l:return static_cast<char>(HTTP::PURGE);
	  } return static_cast<char>(HTTP::InternalMethodCount);
	}
	void _subapi(std::string prefix, const App& subapi) {
	  subapi.map_.for_all_routes([this, prefix](std::string r, VH h) {
		if (!r.empty() && r.back() == '/') h.url = prefix + r; else h.url = prefix + '/' + r;
		this->map_[h.url] = h;
	  });
	}
	//template <typename Adaptor> //websocket
	//void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
	///Process the Req and generate a Res for it
	void _print_routes() {
	  map_.for_all_routes([this](std::string r, VH h) { std::cout << r << '\n'; });
	  std::cout << std::endl;
	}
	void _call(HTTP method, std::string_view route, Req& request, Res& response) const {
	  if (route == last_called_) {
		if (static_cast<char>(method) == last_handler_.verb) {
		  request.url = last_handler_.url; last_handler_.handler(request, response); return;
		} else throw http_error::not_found("Method ", m2c(method), " not implemented on route ", route);
	  }
	  // skip the last / of the url.
	  std::string_view route2(route);
	  if (route2.size() != 0 && route2[route2.size() - 1] == '/') route2 = route2.substr(0, route2.size() - 1);
	  fc::internal::drt_node::iterator it = map_.find(route2); if (it != map_.end()) {
		if (static_cast<char>(method) == it->second.verb) {
		  const_cast<App*>(this)->last_called_ = route;
		  const_cast<App*>(this)->last_handler_ = it->second;
		  request.url = it->second.url;
		  it->second.handler(request, response);
		} else throw http_error::not_found("Method ", m2c(method), " not implemented on route ", route2);
	  } else throw http_error::not_found("Route ", route2, " does not exist.");
	}
	DRT map_; std::string last_called_; VH last_handler_;
  };

} // namespace fc
