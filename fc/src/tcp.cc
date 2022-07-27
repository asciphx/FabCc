#include "tcp.hh"

namespace fc {
  Tcp::Tcp(App* app, uv_loop_t* loop):opened(false), loop_(loop), addr_len(16), app_(app) {
#ifdef _WIN32
	::system("chcp 65001 >nul"); setlocale(LC_CTYPE, ".UTF8");
#else
	std::locale::global(std::locale("en_US.UTF8"));
#endif
	time(&RES_TIME_T); RES_NOW = localtime(&RES_TIME_T); RES_NOW->tm_isdst = 0; RES_last = uv_now(loop);
	RES_DATE_STR.resize(0x30); RES_DATE_STR.resize(strftime(&RES_DATE_STR[0], 0x2f, RES_GMT, RES_NOW));
  }
  Tcp& Tcp::timeout(unsigned short m) {
	keep_milliseconds = m < 301 ? m * 1000 : m < 0x640 ? m *= 6 : m; return *this;
  }
  Tcp& Tcp::directory(std::string p) {
	if (p.back() != '\\' && p.back() != '/') p += '/'; detail::directory_ = p; return *this;
  }
  Tcp& Tcp::upload_path(std::string p) {
	if (p.back() != '\\' && p.back() != '/') p += '/'; detail::upload_path_ = p; return *this;
  }
  Tcp& Tcp::file_type(const std::vector<std::string_view>& line) {
	for (std::vector<std::string_view>::const_iterator iter = line.cbegin(); iter != line.cend(); ++iter) {
	  std::string_view sv; sv = content_any_types[*iter];
	  if (sv != "") { content_types.emplace(*iter, sv); } else { content_types.emplace(*iter, RES_oct); }
	} not_set_types = false; return *this;
  }
  //Tcp& Tcp::home(std::string p) { RES_home = p; return *this; }
  Tcp& Tcp::maxConnection(int backlog) { max_conn = backlog; return *this; }
  Tcp& Tcp::thread(unsigned char n) { max_thread = n; return *this; }//?
  bool Tcp::init() {
	if (opened)return true; opened = true; if (!loop_)return false; if (uv_tcp_init(loop_, &_)) return false; _.data = this; return true;
  }
  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, NULL); uv_stop(loop_); }// uv_loop_close(loop_);
  Tcp::~Tcp() { uv_mutex_destroy(&RES_MUTEX); exit(); }
  Tcp& Tcp::router(App& app) { app_ = &app; return *this; }
  Tcp& Tcp::setTcpNoDelay(bool enable) { uv_tcp_nodelay(&_, enable ? 1 : 0); return *this; }
  bool Tcp::bind(const char* ip_addr, int port, bool is_ipv4) {
	printf("C++ web[服务] run on http://%s:%d", ip_addr, port); int $; if (is_ipv4) {
	  struct sockaddr_in addr; $ = uv_ip4_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = false;
	} else {
	  struct sockaddr_in6 addr; $ = uv_ip6_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = true; addr_len = sizeof(sockaddr_storage);
	} return $ ? true : false;
  }
  bool Tcp::Start(const char* ip_addr, int port, bool is_ipv4) {
	exit(); if (!port)port = port_; if (!init())return false; if (bind(ip_addr, port, is_ipv4))return false;
	if (!fc::is_directory(detail::directory_)) fc::create_directory(detail::directory_); uv_mutex_init_recursive(&RES_MUTEX);
	std::string s(detail::directory_ + detail::upload_path_); if (!fc::is_directory(s)) fc::create_directory(s);
	if (not_set_types) content_types = content_any_types, not_set_types = false;
	//if (not_set_types)file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt" }), not_set_types = false;
	if (uv_listen((uv_stream_t*)&_, max_conn, on_conn))return false; uv_run(loop_, UV_RUN_DEFAULT); uv_loop_close(loop_); return false;
  }
  void Tcp::write_cb(uv_write_t* wr, int st) { Conn* co = (Conn*)wr; uv_close((uv_handle_t*)&co->slot_, on_close); }
  void Tcp::on_read(uv_fs_t* f) {
	Conn* co = (Conn*)f->data; co->file_pos += f->result;
	if (f->result > 0) {
	  //co->body = std::string_view(co->wbuf.base, f->result);
	  //co->buf.base = co->body.data(); co->buf.len = f->result;
	  //printf("Read (%zu): %.*s\n", f->result, (int)f->result, co->wbuf.base);
	  int r = uv_write(&co->_, (uv_stream_t*)&co->slot_, &co->wbuf, 1, [](uv_write_t* wr, int st) {
		Conn* co = (Conn*)wr; if (st) { uv_shutdown(&RES_SHUT_REQ, (uv_stream_t*)&co->slot_, NULL); return; }
		 });
	  if (r) { printf("uv_write error: %s\n", uv_strerror(r)); return; }
	  // if (co->file_pos == co->res_.file_size) {
		 //free(co->wbuf.base);
		 //uv_fs_close(co->loop_, &co->fs_, co->fd_, [](uv_fs_t* f) { uv_fs_req_cleanup(f); });
		 //return;
	  // }
	  uv_fs_req_cleanup(f);
	  uv_fs_read(co->loop_, (uv_fs_t*)&co->fs_, co->fd_, &co->wbuf, 1, co->file_pos, on_read);
	} else {
	  uv_fs_close(co->loop_, &co->fs_, co->fd_, [](uv_fs_t* req) { uv_fs_req_cleanup(req); });
	}
  }
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
	Conn* co = (Conn*)h->data;
	if (nread > 0) {
	  char failed = llhttp__internal_execute(&co->parser_, buf->base, buf->base + nread);
	  if (failed) { uv_close((uv_handle_t*)h, on_close); return; }
	  Req& req = co->req_; req = std::move(co->parser_.to_request());
	  if (!req.url[0] || req.method == HTTP::OPTIONS) return; Res& res = co->res_;
	  LOG_GER(m2c(req.method) << " |" << res.code << "| " << req.url); co->_.data = &res;
	  if (co->req_.keep_alive) {
		res.timer_.setTimeout([h] {
		  uv_shutdown(&RES_SHUT_REQ, h, NULL); uv_close((uv_handle_t*)h, on_close);
		}, co->keep_milliseconds);// res.add_header(RES_Con, "Keep-Alive");
	  } else { co->req_.keep_alive = true; }
	  if (uv_now(co->loop_) - RES_last > 1000) {//uv_mutex_lock(&RES_MUTEX); uv_mutex_unlock(&RES_MUTEX);
		time(&RES_TIME_T); RES_last = uv_now(co->loop_);
#if defined(_MSC_VER) || defined(__MINGW32__)
		localtime_s(RES_NOW, &RES_TIME_T);
#else
		localtime_r(&RES_TIME_T, RES_NOW);
#endif
		RES_DATE_STR.resize(strftime(&RES_DATE_STR[0], 0x2f, RES_GMT, RES_NOW));
	  } //printf("<%s,%lld> ", req.params.c_str(), req.uuid);
	  fc::Buffer& s = co->buf_;
	  try {
		failed = co->app_->_call(req.method, req.url, req, res); if (failed) res.set_static_file_info(req.url);
		co->set_status(res, res.code);
		if (res.is_file > 0) {
		  s << RES_http_status << co->status_;
		  for (std::pair<const std::string, std::string>& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
#if SHOW_SERVER_NAME
		  s << RES_server_tag << SERVER_NAME << RES_crlf;
#endif
		  if (res.is_file == 1) {
			res.is_file = 0;
			req.uuid = hackUrl(req.url.c_str());
			if (RES_CACHE_TIME[req.uuid] > nowStamp()) {
			  res.body = RES_CACHE_MENU[req.uuid]; goto _;
			}
			RES_CACHE_TIME[req.uuid] = nowStamp(CACHE_HTML_TIME_SECOND);
			s << RES_CT << RES_seperator << RES_Txt << RES_crlf;
			s << RES_content_length_tag << std::to_string(res.file_size) << RES_crlf;
			s << RES_date_tag << RES_DATE_STR << RES_crlf;
			s << RES_crlf;
			if (!co->write(s.data_, s.size())) { return; }; s.clear();
			std::ifstream inf(res.path_, std::ios::in | std::ios::binary);
			int l = 0, i; $:l = inf.read(co->readbuf, BUF_SIZE).gcount();
			if (l) {
			  i = ::send(co->id, co->readbuf, l, 0);
			  res.body += std::string_view(co->readbuf, l); if (i > 0) goto $;
			}
			inf.close(); RES_CACHE_MENU[req.uuid] = std::move(res.body);
			return;
		  }
		  s << RES_date_tag << RES_DATE_STR << RES_crlf;
		  res.is_file = 0; res.headers.clear(); res.body.clear();
		  s << RES_Ca << RES_seperator << FILE_TIME << RES_crlf << RES_Xc << RES_seperator << RES_No << RES_crlf;
		  s << RES_crlf; if (!co->write(s.data_, s.size())) { return; } s.clear();
		  std::ifstream inf(res.path_, std::ios::in | std::ios::binary);
		  int l = 0, i; __:l = inf.read(co->readbuf, BUF_SIZE).gcount();
		  if (l) { i = ::send(co->id, co->readbuf, l, 0); if (i > 0) goto __; }
		  inf.close(); std::this_thread::yield();
		  return;
		}
	  } catch (const http_error& e) {
		co->set_status(res, e.i()); res.body += e.what(); s << RES_http_status << co->status_; goto _;
	  } catch (const std::runtime_error& e) {
		co->set_status(res, 500); res.body = e.what(); s << RES_http_status << co->status_; goto _;
	  }
	  //if (fc::KEY_EQUALS(fc::get_header(req.headers, RES_Ex), "100-continue") &&
	  // co->parser_.http_major == 1 && co->parser_.http_minor == 1)s << expect_100_continue;
	  s << RES_http_status << co->status_;
	  for (std::pair<const std::string, std::string>& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
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
#if SHOW_SERVER_NAME
	  s << RES_server_tag << SERVER_NAME << RES_crlf;
#endif
	_:
	  if (!res.headers.count(RES_CT)) {
		s << RES_CT << RES_seperator << RES_Txt << RES_crlf;
	  }
	  s << RES_content_length_tag << std::to_string(res.body.size()) << RES_crlf;
	  s << RES_date_tag << RES_DATE_STR << RES_crlf;
	  s << RES_crlf << res.body; res.headers.clear(); res.code = 200; res.body.clear();
	  DEBUG("客户端：%lld %s \n", co->id, s.c_str());
	  co->write(s.data_, s.size()); s.clear();
	  //co->wbuf.base = s.data_; co->wbuf.len = s.size();
	  //int r = uv_write(&co->_, h, &co->wbuf, 1, NULL); s.clear();
	  //if (r) { DEBUG("uv_write error: %s\n", uv_strerror(r)); return; }
	} else if (nread < 0) {
	  if (nread == UV_EOF || nread == UV_ECONNRESET) {
		DEBUG("1->%Id: %s : %s\n", nread, uv_err_name(nread), uv_strerror(nread));
		uv_close((uv_handle_t*)h, on_close);
	  } else {//UV_ENOBUFS
		DEBUG("2->%Id: %s : % s\n", nread, uv_err_name(nread), uv_strerror(nread));
		uv_shutdown(&RES_SHUT_REQ, h, NULL); uv_close((uv_handle_t*)h, on_close);
	  }//if (!uv_is_active((uv_handle_t*)h))// uv_read_stop((uv_stream_t*)h);
	}//printf("%d\n", nread);uv_read_stop(h);
  }
  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested, uv_buf_t* b) { Conn* c = (Conn*)h->data; *b = c->rbuf; }
  void Tcp::on_close(uv_handle_t* h) {
	Conn* c = (Conn*)h->data; --((Tcp*)c->tcp_)->connection_num; DEBUG("{x%Id}\n", c->id); delete c;
  }
  void Tcp::on_conn(uv_stream_t* srv, int st) {
	Tcp* t = (Tcp*)srv->data; if (t->connection_num > t->max_conn) return;
	Conn* co = new Conn(t->keep_milliseconds, t->loop_, &t->content_types);
	int $ = uv_tcp_init(t->loop_, &co->slot_); if ($) { delete co; return; }
	$ = uv_accept((uv_stream_t*)&t->_, (uv_stream_t*)&co->slot_);
	if ($) { uv_close((uv_handle_t*)&co->slot_, NULL); delete co; return; }
	co->req_.ip_addr.resize(t->addr_len); co->app_ = t->app_; co->tcp_ = t;
	if (0 == uv_tcp_getpeername((uv_tcp_t*)&co->slot_, (sockaddr*)&t->addr_, &t->addr_len)) {
	  if (!t->is_ipv6) {
		sockaddr_in addrin = *((sockaddr_in*)&t->addr_);
		uv_ip4_name(&addrin, (char*)co->req_.ip_addr.data(), t->addr_len);
	  } else {
		sockaddr_in6 addrin = *((sockaddr_in6*)&t->addr_);
		uv_ip6_name(&addrin, (char*)co->req_.ip_addr.data(), t->addr_len);
	  }
	  DEBUG(" %s:%d\n", co->req_.ip_addr.c_str(), ntohs(co->id));
	}
	co->id = co->slot_.socket; co->set_keep_alive(co->id, 3, 3, 2); ++t->connection_num;
	uv_read_start((uv_stream_t*)&co->slot_, alloc_cb, read_cb);//uv_tcp_keepalive(&co->slot_, 1, 6);
  }
}
