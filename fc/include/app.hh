#pragma once
#include <functional>
#include <iostream>
#include <thread>
#include <atomic>
#include <h/common.h>
#include "router.hh"
#include "hpp/http_error.hpp"
#include <buffer.hh>
#include <conn.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
#define CTX_BUF_SIZE 0x10000
namespace fc {
  char sv2c(const char* m);
  const char* m2c(HTTP m);
  struct Ctx {
	uv_write_t _;
	uv_fs_t fs_;
	uv_file fd_;
	int64_t file_pos = 0, file_size = 0;
	uv_buf_t rbuf, wbuf;
	bool rd_ = true;
	//uv_idle_t idler;
	Ctx() { rbuf = uv_buf_init((char*)malloc(CTX_BUF_SIZE), CTX_BUF_SIZE); this->fs_.data = this; }
	std::function<void(void*)> func;//uv_idle_init(co->loop_, &this->idler); this->idler.data = this;
	void reset() {
	  file_pos = file_size = 0; rd_ = true;
	}
	~Ctx() { free(rbuf.base); }
  };
  //uv_idle_start(&this->idler, [](uv_idle_t* h) {
  //  uv_idle_stop(h);
  //  }); uv_idle_t idler; uv_idle_init(uv_default_loop(), &this->idler); this->idler.data = this;
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
	Ctx context_[6];
	std::atomic<unsigned char> ctx_idex = 0xff;
	_INLINE Ctx* context() {
	_:Ctx* l = &context_[++ctx_idex]; if (ctx_idex == 5) ctx_idex = 0xff;
	  if (l->rd_) {
		l->rd_ = false;
		return l;
	  } std::this_thread::yield(); goto _;
	}
  };
} // namespace fc
