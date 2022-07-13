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
	VH& operator[](const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::GET)); return h;
	}
	VH& del(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::DEL)); return h;
	}
	VH& get(const char* r = "/") {
	  VH& h = map_.add(r, static_cast<char>(HTTP::GET)); return h;
	}
	VH& post(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::POST)); return h;
	}
	VH& put(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::PUT)); return h;
	}
	VH& patch(const char* r) {
	  VH& h = map_.add(r, static_cast<char>(HTTP::PATCH)); return h;
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
	  Buffer b(0x1ff); int i = 0, m = 1; map_.for_all_routes([this, &b, &i, &m](std::string r, VH h) {
		m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
		b << '(' << ++i << ')' << '[' << m2c((HTTP)m) << ']' <<
		  '/' << (r[2] == 0x2f ? r.substr(3) : r.substr(2)) << ',' << (i % 6 == 0 ? "<br/>" : " ");
		}); return b;
	}
	void _call(HTTP& m, std::string& r, Req& request, Res& response) const {
	  if (r.size() != 1 && r[r.size() - 1] == '/') r = r.substr(0, r.size() - 1);
	  if (r == last_called_) {// skip the last / of the url.
		last_handler_(request, response); return;
	  } std::string g; static_cast<char>(m) > 9 ? g.push_back(static_cast<char>(m) % 10 + 0x30),
		g.push_back(static_cast<char>(m) / 10 + 0x30) : g.push_back(static_cast<char>(m) + 0x30); g += r;
	  fc::drt_node::iterator it = map_.find(g); if (it != map_.end()) {
		const_cast<App*>(this)->last_called_ = r;
		const_cast<App*>(this)->last_handler_ = it->second;
		it->second(request, response);
	  } else throw http_error::not_found("Route ", r, " does not exist.");
	}
	DRT map_; std::string last_called_; VH last_handler_;
  };
} // namespace fc
