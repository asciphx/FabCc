#include "tcp.hh"
namespace fc {
  Tcp::Tcp(App* app, uv_loop_t* loop):opened(false), loop_(loop), addr_len(16), app_(app) {}
  Tcp& Tcp::thread(unsigned char n) { num = n; return *this; }//?
  bool Tcp::init() {
	if (opened)return true; opened = true; if (!loop_)return false;
	if (uv_tcp_init(loop_, &_)) return false; _.data = this; return true;
  }
  void Tcp::exit() { if (!opened)return; opened = false; uv_stop(loop_); }// uv_loop_close(loop_);
  Tcp::~Tcp() { uv_close((uv_handle_t*)&_, on_exit); exit(); }
  Tcp& Tcp::router(App& app) { app_ = &app; return *this; }
  Tcp& Tcp::setTcpNoDelay(bool enable) { uv_tcp_nodelay(&_, enable ? 1 : 0); return *this; }
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
	if (listen())return false; uv_run(loop_, UV_RUN_DEFAULT); return false;
  }
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
	Conn* co = (Conn*)h->data; if (co == nullptr) return;
	//if (h->write_queue_size < co->_.u.io.queued_bytes) { uv_read_stop(h); co->_.u.io.queued_bytes = 0; delete co; return; }
	if (nread > 0) {
	  bool failed = llhttp__internal_execute(&co->parser_, buf->base, buf->base + nread);
	  if (failed) { uv_close((uv_handle_t*)h, on_close); return; } std::string& s = co->buf_;
	  if (co->req_.keep_alive) co->res_.add_header(RES_Con, "Keep-Alive");
	  Req& req = co->req_; req.method = static_cast<HTTP>(co->parser_.method);
	  req.url = co->parser_.url.data(); req.raw_url = std::move(co->parser_.raw_url);
	  req.body = co->parser_.body.data(); req.headers = std::move(co->parser_.headers);
	  if (!co->req_.keep_alive)printf("<%d>", co->id); req.keep_alive = co->parser_.keep_alive;
	  //if (fc::KEY_EQUALS(fc::get_header(req.headers, RES_Con), "")) {
	  //}
	  try {
		co->app_->_call(req.method, req.url, req, co->res_);
		co->set_status(co->res_, co->res_.code);
	  } catch (const http_error& e) {
		co->set_status(co->res_, e.status()); co->res_.body = e.what();
		//uv_read_stop(h); delete co; return;
	  } catch (const std::runtime_error& e) {
		co->set_status(co->res_, 500); co->res_.body = e.what();
	  }
	  if (fc::KEY_EQUALS(fc::get_header(req.headers, RES_Ex), "100-continue") &&
		co->parser_.http_major == 1 && co->parser_.http_minor == 1) s += expect_100_continue;
	  s += RES_http_status; s += co->status_;
	  for (auto& kv : co->res_.headers) s += kv.first, s += RES_seperator, s += kv.second, s += RES_crlf;
#ifdef AccessControlAllowCredentials
	  s += RES_AcC; s += AccessControlAllowCredentials; s += RES_crlf;
#endif
#ifdef AccessControlAllowHeaders
	  s += RES_AcH; s += AccessControlAllowHeaders; s += RES_crlf;
#endif
#ifdef AccessControlAllowMethods
	  s += RES_AcM; s += AccessControlAllowMethods; s += RES_crlf;
#endif
#ifdef AccessControlAllowOrigin
	  s += RES_AcO; s += AccessControlAllowOrigin; s += RES_crlf;
#endif
	  if (!co->res_.headers.count(RES_CT)) {
		s += RES_CT; s += RES_seperator; s += RES_Txt; s += RES_crlf;
	  }
	  s += RES_content_length_tag; s += std::to_string(co->res_.body.size()); s += RES_crlf;
#if SHOW_SERVER_NAME
	  s += RES_server_tag; s += SERVER_NAME; s += RES_crlf;
#endif
	  s += RES_crlf; s += co->res_.body;
	  DEBUG("客户端：%d %s \n", co->id, s.c_str());
	  co->wbuf.base = s.data(); co->wbuf.len = s.size();
	  int r = uv_write(&co->_, h, &co->wbuf, 1, NULL); s.clear();//write_cb
	  co->res_.body.clear(); co->res_.code = 200; co->res_.headers.clear();
	  if (r) { DEBUG("uv_write error: %s\n", uv_strerror(r)); return; }
	} else if (nread < 0) {
	  if (nread == UV_EOF || nread == UV_ECONNRESET) {
		uv_close((uv_handle_t*)&co->slot_, on_close);
	  } else {//UV_ENOBUFS
		DEBUG("name: %s err: %s\n", uv_err_name(nread), uv_strerror(nread));
		uv_read_stop(h);
	  }//if (!uv_is_active((uv_handle_t*)&co->slot_))// uv_read_stop((uv_stream_t*)&co->slot_);
	}//printf("%d\n", nread);
  }
  void Tcp::write_cb(uv_write_t* wr, int status) {
	Conn* co = (Conn*)wr; if (status) { uv_close((uv_handle_t*)&co->slot_, on_close); return; };
  }
  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested, uv_buf_t* b) {
	Conn* c = (Conn*)h->data; if (c->rbuf.len < suggested) {
	  if (c->rbuf.base != nullptr)free(c->rbuf.base);
	  c->rbuf.base = (char*)malloc(suggested); c->rbuf.len = suggested;
	} *b = c->rbuf;
  }
  void Tcp::on_close(uv_handle_t* h) {
	Conn* c = (Conn*)h->data; printf("{%d} x！ \n", c->id); delete c;
  }
  void Tcp::on_connection(uv_stream_t* srv, int status) {
	Tcp* tcp = (Tcp*)srv->data; Conn* co = new Conn();
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
	co->id = co->slot_.socket; co->set_keep_alive(co->id, 5, 4, 3);
	uv_read_start((uv_stream_t*)&co->slot_, alloc_cb, read_cb);// uv_tcp_keepalive(&co->slot_, 1, 6);
  }
}
