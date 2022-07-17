#include "tcp.hh"
namespace fc {
  Tcp::Tcp(App* app, uv_loop_t* loop):opened(false), loop_(loop), addr_len(16), app_(app) {}
  Tcp& Tcp::thread(unsigned char n) { num = n; return *this; }//?
  bool Tcp::init() {
	if (opened)return true; opened = true; if (!loop_)return false;
	if (uv_tcp_init(loop_, &_)) return false; _.data = this; return true;
  }
  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, NULL); uv_stop(loop_); }// uv_loop_close(loop_);
  Tcp::~Tcp() { exit(); }
  Tcp& Tcp::router(App& app) { app_ = &app; return *this; }
  Tcp& Tcp::setTcpNoDelay(bool enable) { uv_tcp_nodelay(&_, enable ? 1 : 0); return *this; }
  Tcp& Tcp::timeout(unsigned short m) { keep_milliseconds = m < 601 ? m * 1000 : m; return *this; }
  bool Tcp::bind(const char* ip_addr, int port, bool is_ipv4) {
	int $; if (is_ipv4) {
	  struct sockaddr_in addr; $ = uv_ip4_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = false;
	} else {
	  struct sockaddr_in6 addr; $ = uv_ip6_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = true; addr_len = sizeof(sockaddr_storage);
	} return $ ? true : false;
  }
  bool Tcp::listen(int i) { return uv_listen((uv_stream_t*)&_, i, on_connection) ? true : false; }
  bool Tcp::Start(const char* ip_addr, int port, bool is_ipv4) {
	exit(); if (!port)port = port_; if (!init())return false; if (bind(ip_addr, port, is_ipv4))return false;
	if (listen())return false; uv_run(loop_, UV_RUN_DEFAULT); uv_loop_close(loop_); return false;
  }
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
	Conn* co = (Conn*)h->data;//uv_shutdown(&shutdown_req, h, NULL);
	if (nread > 0) {
	  int failed = llhttp__internal_execute(&co->parser_, buf->base, buf->base + nread);
	  if (failed) { uv_close((uv_handle_t*)h, on_close); return; } Res& res = co->res_;
	  if (co->req_.keep_alive) res.add_header(RES_Con, "Keep-Alive");
	  Req& req = co->req_; req.method = static_cast<HTTP>(co->parser_.method);
	  req.url = co->parser_.url.data(); req.params = std::move(co->parser_.url_params);
	  req.body = co->parser_.body.data(); req.headers = std::move(co->parser_.headers);
	  req.uuid = hackUrl(req.url.c_str()); fc::Buffer& s = co->buf_;
	  //printf("<%s,%lld> ", req.params.c_str(), req.uuid);
	  try {
		co->app_->_call(req.method, req.url, req, res); co->set_status(res, res.code);
		co->req_.keep_alive = true;
		res.timer_.setTimeout([h] {
		  uv_shutdown(&RES_SHUT_REQ, h, NULL); uv_close((uv_handle_t*)h, on_close);
		}, co->keep_milliseconds);
	  } catch (const http_error& e) {
		co->set_status(res, e.status()); res.body = e.what(); //uv_read_stop(h); delete co; return;
	  } catch (const std::runtime_error& e) {
		co->set_status(res, 500); res.body = e.what();
	  } if (fc::KEY_EQUALS(fc::get_header(req.headers, RES_Ex), "100-continue") &&
		co->parser_.http_major == 1 && co->parser_.http_minor == 1) s << expect_100_continue;
	  s << RES_http_status << co->status_;
	  for (auto& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
#ifdef AccessControlAllowCredentials
	  s << RES_AcC << AccessControlAllowCredentials << RES_crlf;
#endif
#ifdef AccessControlAllowHeaders
	  s << RES_AcH << AccessControlAllowHeaders << RES_crlf;
#endif
#ifdef AccessControlAllowMethods
	  s << RES_AcM << AccessControlAllowMethods << RES_crlf;
#endif
#ifdef AccessControlAllowOrigin
	  s << RES_AcO << AccessControlAllowOrigin << RES_crlf;
#endif
	  if (!res.headers.count(RES_CT)) {
		s << RES_CT << RES_seperator << RES_Txt << RES_crlf;
	  }
	  s << RES_content_length_tag << std::to_string(res.body.size()) << RES_crlf;
#if SHOW_SERVER_NAME
	  s << RES_server_tag << SERVER_NAME << RES_crlf;
#endif
	  s << RES_crlf << res.body; res.headers.clear(); res.code = 200;
	  DEBUG("客户端：%d %s \n", co->id, s.c_str()); res.body.clear();
	  co->wbuf.base = s.data_; co->wbuf.len = s.size();
	  int r = uv_write(&co->_, h, &co->wbuf, 1, NULL); s.clear();
	  if (r) { DEBUG("uv_write error: %s\n", uv_strerror(r)); return; }
	} else if (nread < 0) {
	  if (nread == UV_EOF || nread == UV_ECONNRESET) {
		DEBUG("1->%ld: %s : %s\n", nread, uv_err_name(nread), uv_strerror(nread));
		uv_close((uv_handle_t*)h, on_close);
	  } else {//UV_ENOBUFS
		DEBUG("2->%ld: %s : % s\n", nread, uv_err_name(nread), uv_strerror(nread));
		uv_shutdown(&RES_SHUT_REQ, h, NULL); uv_close((uv_handle_t*)h, on_close);
	  }//if (!uv_is_active((uv_handle_t*)h))// uv_read_stop((uv_stream_t*)h);
	}//printf("%d\n", nread);uv_read_stop(h);
  }
  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested, uv_buf_t* b) {
	Conn* c = (Conn*)h->data; *b = c->rbuf;// b->base = (char*)malloc(suggested); b->len= suggested;
  }
  void Tcp::on_close(uv_handle_t* h) { Conn* c = (Conn*)h->data; DEBUG("{%d}x！\n", c->id); delete c; }
  void Tcp::on_connection(uv_stream_t* srv, int status) {
	Tcp* tcp = (Tcp*)srv->data; Conn* co = new Conn(tcp->keep_milliseconds);
	int $ = uv_tcp_init(tcp->loop_, &co->slot_); if ($) { delete co; return; }
	$ = uv_accept((uv_stream_t*)&tcp->_, (uv_stream_t*)&co->slot_);
	if ($) { uv_close((uv_handle_t*)&co->slot_, NULL); delete co; return; }
	co->req_.ip_addr.resize(tcp->addr_len); co->app_ = tcp->app_;
	if (0 == uv_tcp_getpeername((uv_tcp_t*)&co->slot_, (sockaddr*)&tcp->addr_, &tcp->addr_len)) {
	  if (!tcp->is_ipv6) {
		sockaddr_in addrin = *((sockaddr_in*)&tcp->addr_);
		uv_ip4_name(&addrin, (char*)co->req_.ip_addr.data(), tcp->addr_len);
	  } else {
		sockaddr_in6 addrin = *((sockaddr_in6*)&tcp->addr_);
		uv_ip6_name(&addrin, (char*)co->req_.ip_addr.data(), tcp->addr_len);

	  }
	  DEBUG(" %s:%d\n", co->req_.ip_addr.c_str(), ntohs(co->id));
	}
	co->id = co->slot_.socket; co->set_keep_alive(co->id, 3, 3, 2);
	uv_read_start((uv_stream_t*)&co->slot_, alloc_cb, read_cb);//uv_tcp_keepalive(&co->slot_, 1, 6);
  }
}
