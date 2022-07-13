#pragma once
#include <functional>
#include <iostream>
#include <h/common.h>
#include "router.hh"
#include "http_error.hh"
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
namespace fc {
  struct App {
	App() {}
	std::function<void(Req&, Res&)>& operator[](const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::GET)); h.verb = static_cast<char>(HTTP::GET); return h.handler;
	}
	std::function<void(Req&, Res&)>& del(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::DEL));  h.verb = static_cast<char>(HTTP::DEL); return h.handler;
	}
	std::function<void(Req&, Res&)>& get(const char* r = "/") {
	  VH& h = map_.add(r, static_cast<char>(HTTP::GET));  h.verb = static_cast<char>(HTTP::GET); return h.handler;
	}
	std::function<void(Req&, Res&)>& post(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::POST));  h.verb = static_cast<char>(HTTP::POST); return h.handler;
	}
	std::function<void(Req&, Res&)>& put(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::PUT));  h.verb = static_cast<char>(HTTP::PUT); return h.handler;
	}
	std::function<void(Req&, Res&)>& patch(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::PATCH));  h.verb = static_cast<char>(HTTP::PATCH); return h.handler;
	}
	char sv2c(const char* m) const {
	  switch (hack8Str(m)) {
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
	  } return static_cast<char>(HTTP::INVALID);
	}
	//template <typename Adaptor> //websocket
	//void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
	///Process the Req and generate a Res for it
	Buffer _print_routes() {
	  Buffer b(0x1ff); int i = 0; map_.for_all_routes([this, &b, &i](std::string r, VH h) {
		b << '(' << ++i << ')' << '[' << m2c((HTTP)h.verb) << ']' <<
		  '/' << (h.verb < 10 ? r.substr(2) : r.substr(3)) << ',' << (i % 6 == 0 ? "<br/>" : " ");
		}); return b;
	}
	void _call(HTTP m, std::string& route, Req& request, Res& response) const {
	  if (route.size() != 1 && route[route.size() - 1] == '/') route = route.substr(0, route.size() - 1);
	  if (route == last_called_) {// skip the last / of the url.
		if (static_cast<char>(m) == last_handler_.verb) {
		  last_handler_.handler(request, response); return;
		} else throw http_error::not_found("Method ", m2c(m), " not implemented on route ", route);
	  } std::string route2; static_cast<char>(m) > 9 ? route2.push_back(static_cast<char>(m) % 10 + 0x30),
		route2.push_back(static_cast<char>(m) / 10 + 0x30) : route2.push_back(static_cast<char>(m) + 0x30); route2 += route;
	  fc::drt_node::iterator it = map_.find(route2); if (it != map_.end()) {
		if (static_cast<char>(m) == it->second.verb) {
		  const_cast<App*>(this)->last_called_ = route;
		  const_cast<App*>(this)->last_handler_ = it->second;
		  it->second.handler(request, response);
		} else throw http_error::not_found("Method ", m2c(m), " not implemented on route ", route2);
	  } else throw http_error::not_found("Route ", route, " does not exist.");
	}
	DRT map_; std::string last_called_; VH last_handler_;
  };
} // namespace fc
