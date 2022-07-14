#pragma once
#include <functional>
#include <iostream>
#include <h/common.h>
#include "router.hh"
#include "hpp/http_error.hpp"
#include <buffer.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
namespace fc {
  struct App {
	App();
	VH& operator[](const char* r);
	VH& del(const char* r);
	VH& get(const char* r = "/");
	VH& post(const char* r);
	VH& put(const char* r);
	VH& patch(const char* r);
	char sv2c(const char* m) const;
	//template <typename Adaptor> //websocket
	//void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
	///Process the Req and generate a Res for it
	Buffer _print_routes();
	void _call(HTTP& m, std::string& r, Req& request, Res& response) const;
	DRT map_; std::string last_called_; VH last_handler_;
  };
} // namespace fc
