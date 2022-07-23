#pragma once
#include <functional>
#include <iostream>
#include <h/common.h>
#include "router.hh"
#include "hpp/http_error.hpp"
#include <buffer.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
namespace fc {
  char sv2c(const char* m);
  const char* m2c(HTTP m);
  struct App {
	App();
	VH& operator[](const char* r);
	VH& del(const char* r);
	VH& get(const char* r = "/");//"\0"//with skip the last / of the url.
	VH& post(const char* r);
	VH& put(const char* r);
	VH& patch(const char* r);
	//template <typename Adaptor> //websocket
	//void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
	///Process the Req and generate a Res for it
	Buffer _print_routes();
	char _call(HTTP& m, std::string& r, Req& request, Res& response) const;
	DRT map_;// VH last_handler_; std::string last_called_;
  };
} // namespace fc
