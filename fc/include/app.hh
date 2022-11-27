#pragma once
#include <functional>
#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include <h/common.h>
#include <h/any_types.h>
#include "router.hh"
#include <buf.hh>
#include <directory.hh>
#include <http_error.hh>
#include <file_sptr.hh>
namespace fc {
  static std::unordered_map<uint64_t, fc::Buf> RES_CACHE_MENU = {};
  static std::unordered_map<uint64_t, int64_t> RES_CACHE_TIME = {};
  static volatile int quit_signal_catched = 0;
  char c2m(const char* m);
  const char* m2c(HTTP m);
  struct App {
	App();
	VH& operator[](const char* r);
	VH& del(const char* r);
	VH& get(const char* r = "/");//"\0"//with skip the last / of the url.
	VH& post(const char* r);
	VH& put(const char* r);
	//VH& patch(const char* r);
	//template <typename Adaptor> //websocket
	//void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
	///Process the Req and generate a Res for it
	Buf _print_routes();
	void _call(char m, std::string& r, Req& request, Res& response) const;
	DRT map_;
	std::unordered_map<std::string, std::shared_ptr<file_sptr>> file_cache_;
    //std::unordered_map<std::string_view, std::string_view> content_types;
	void sub_api(const char* prefix, const App& subapi);
	//Serve static directory
	App serve_file(const char* r);
	App& file_type(const std::vector<std::string_view>& line = { "html","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4" });
  };
  void http_serve(App& api, int port = 8080, int nthreads = std::thread::hardware_concurrency(), std::string ip = "");
} // namespace fc
