#include "tcp.hh"

namespace fc {
  Tcp::Tcp(App* app, uv_loop_t* loop):opened(false), loop_(loop), addr_len(16), app_(app) {
#ifdef _WIN32
	::system("chcp 65001 >nul"); setlocale(LC_CTYPE, ".UTF8");
#else
	std::locale::global(std::locale("en_US.UTF8"));
#endif
	//std::time(&RES_TIME_T); RES_NOW = std::localtime(&RES_TIME_T); RES_NOW->tm_isdst = 0;
  }
  Tcp& Tcp::thread(unsigned char n) { num = n; return *this; }//?
  bool Tcp::init() {
	if (opened)return true; opened = true; if (!loop_)return false;
	if (uv_tcp_init(loop_, &_)) return false; _.data = this; return true;
  }
  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, NULL); uv_stop(loop_); }// uv_loop_close(loop_);
  Tcp::~Tcp() { exit(); if (app_ != nullptr)app_ = nullptr; }
  Tcp& Tcp::router(App& app) { app_ = &app; return *this; }
  Tcp& Tcp::setTcpNoDelay(bool enable) { uv_tcp_nodelay(&_, enable ? 1 : 0); return *this; }
  Tcp& Tcp::timeout(unsigned short m) { keep_milliseconds = m < 601 ? m * 1000 : m; return *this; }
  bool Tcp::bind(const char* ip_addr, int port, bool is_ipv4) {
	printf("C++ web[服务] run on http://localhost:%d", port);
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
	if (!std::filesystem::is_directory(detail::directory_)) std::filesystem::create_directory(detail::directory_);
	std::string s(detail::directory_ + detail::upload_path_); if (!std::filesystem::is_directory(s)) std::filesystem::create_directory(s);
	if (listen())return false; uv_run(loop_, UV_RUN_DEFAULT); uv_loop_close(loop_); return false;
  }
  void Tcp::write_cb(uv_write_t* wr, int st) {
	Conn* co = (Conn*)wr; uv_close((uv_handle_t*)&co->slot_, on_close);
  }
  void Tcp::on_read(uv_fs_t* req) {
	Conn* co = (Conn*)req->data; co->rbuf.base[req->result] = 0;
	//printf("!%d!", req->result);
	co->buf_ << std::string_view(co->rbuf.base, req->result);
	co->wbuf.base = co->buf_.data_; co->wbuf.len = co->buf_.size();
	uv_fs_close(co->loop_, &co->fs_, co->fd_, [](uv_fs_t* req) { uv_fs_req_cleanup(req); });
	int r = uv_write(&co->_, (uv_stream_t*)&co->slot_, &co->wbuf, 1, [](uv_write_t* wr, int st) {
	  Conn* co = (Conn*)wr; if (st) { uv_shutdown(&RES_SHUT_REQ, (uv_stream_t*)&co->slot_, NULL); return; }
	   }); co->buf_.clear();
	   if (r) { printf("uv_write error: %s\n", uv_strerror(r)); return; }
  }
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
	Conn* co = (Conn*)h->data;//uv_shutdown(&shutdown_req, h, NULL);
	if (nread > 0) {
	  char failed = llhttp__internal_execute(&co->parser_, buf->base, buf->base + nread);
	  if (failed) { uv_close((uv_handle_t*)h, on_close); return; } Res& res = co->res_;
	  Req& req = co->req_; req.method = static_cast<HTTP>(co->parser_.method); res.code = 200;
	  req.url = std::move(co->parser_.url); req.params = std::move(co->parser_.url_params);
	  req.body = std::move(co->parser_.body); req.headers = std::move(co->parser_.headers);
	  if (co->req_.keep_alive) {
		res.add_header(RES_Con, "Keep-Alive"); res.timer_.setTimeout([h] {
		  uv_shutdown(&RES_SHUT_REQ, h, NULL); uv_close((uv_handle_t*)h, on_close);
		}, co->keep_milliseconds);
	  } else { co->req_.keep_alive = true; req.uuid = hackUrl(req.url.c_str()); }
	  fc::Buffer& s = co->buf_; //printf("<%s,%lld> ", req.params.c_str(), req.uuid);
	  try {
		failed = co->app_->_call(req.method, req.url, req, res);
		if (failed) {
		  res.set_static_file_info(req.url);
		  DEBUG(" code: %d |", res.code);
		} co->set_status(res, res.code);
		if (res.is_file > 0) {
		  s << RES_http_status << co->status_;
		  for (auto& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
#if SHOW_SERVER_NAME
		  s << RES_server_tag << SERVER_NAME << RES_crlf;
#endif
		  res.headers.clear(); res.body.clear();
		  res.is_file = 0;
		  s << RES_Ca << RES_seperator << FILE_TIME << RES_crlf << RES_Xc << RES_seperator << RES_No << RES_crlf;
		  s << RES_crlf; co->fs_.data = co;
		  co->fd_ = uv_fs_open(co->loop_, &co->ofs_, res.path_.c_str(), O_RDONLY | O_SEQUENTIAL, 0, nullptr);
		  uv_fs_req_cleanup(&co->ofs_); uv_fs_read(co->loop_, (uv_fs_t*)&co->fs_, co->fd_, &co->rbuf, 1, 0, on_read);
		  return;
		  // if (res.is_file == 2) {
			 //printf("<%s>", res.body.c_str());
			 //res.is_file = 0; s.clear();
			 //return;
		  // }
		  //if (res.is_file == 1) {
		  //}
		}
	  } catch (const http_error& e) {
		co->set_status(res, e.status()); res.body = e.what(); co->req_.keep_alive = false;
	  } catch (const std::runtime_error& e) {
		co->set_status(res, 500); res.body = e.what(); co->req_.keep_alive = false;
	  } if (fc::KEY_EQUALS(fc::get_header(req.headers, RES_Ex), "100-continue") &&
		co->parser_.http_major == 1 && co->parser_.http_minor == 1) s << expect_100_continue;
	  s << RES_http_status << co->status_;
	  for (auto& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
#if SHOW_SERVER_NAME
	  s << RES_server_tag << SERVER_NAME << RES_crlf;
#endif
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
	  s << RES_crlf << res.body; res.headers.clear();
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
  void Tcp::on_connection(uv_stream_t* srv, int st) {
	Tcp* tcp = (Tcp*)srv->data; Conn* co = new Conn(tcp->keep_milliseconds, tcp->loop_);
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
