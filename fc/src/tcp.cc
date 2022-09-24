#include "tcp.hh"
#include <lexical_cast.hh>
#pragma warning(disable: 4996)
namespace fc {
  Tcp::Tcp(App* app, uv_loop_t* loop):opened(false), loop_(loop), addr_len(16), app_(app) {
#ifdef _WIN32
	::system("chcp 65001 >nul"); setlocale(LC_CTYPE, ".UTF8");
#else
	std::locale::global(std::locale("en_US.UTF8"));
#endif
	time(&RES_TIME_T); RES_NOW = localtime(&RES_TIME_T); RES_NOW->tm_isdst = 0; RES_last = uv_now(loop);
	RES_DATE_STR.resize(0x30); RES_DATE_STR.resize(strftime(&RES_DATE_STR[0], 0x2f, RES_GMT, RES_NOW));
	if (app_ != nullptr) { app_->content_types = &content_types; } _.data = this;
  }
  Tcp& Tcp::timeout(unsigned short m) {
	keep_milliseconds = m < 301 ? m * 1000 : m < 0x640 ? m *= 6 : m; return *this;
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
  Tcp& Tcp::setThread(char n) {
	uv_cpu_info_t* uc; int cpu; uv_cpu_info(&uc, &cpu); uv_free_cpu_info(uc, cpu); threads = n > 0 ? n : 2 * cpu; return *this;
  }
  Tcp& Tcp::maxConnection(int backlog) { max_conn = backlog < 0 ? 1 : backlog; return *this; }
  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, NULL); uv_stop(loop_); }// uv_loop_close(loop_);
  Tcp::~Tcp() { uv_mutex_destroy(&RES_MUTEX); exit(); }
  Tcp& Tcp::router(App& app) { app_ = &app; app_->content_types = &content_types; return *this; }
  Tcp& Tcp::setTcpNoDelay(bool enable) { uv_tcp_nodelay(&_, enable ? 1 : 0); return *this; }
  bool Tcp::bind(const char* ip_addr, int port, bool is_ipv4) {
	std::cout << "C++<web> => http://127.0.0.1:" << port << std::endl; int $; if (is_ipv4) {
	  struct sockaddr_in addr; $ = uv_ip4_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = false;
	} else {
	  struct sockaddr_in6 addr; $ = uv_ip6_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = true; addr_len = sizeof(sockaddr_storage);
	} return $ ? true : false;
  }
  bool Tcp::Start(const char* ip_addr, int port, bool is_ipv4) {
	exit(); opened = true; if (uv_tcp_init(loop_, &_)) return false;
	if (!port)port = port_; if (bind(ip_addr, port, is_ipv4))return false;
	if (!is_directory(detail::directory_)) create_directory(detail::directory_); uv_mutex_init_recursive(&RES_MUTEX);
	std::string s(detail::directory_ + detail::upload_path_); if (!is_directory(s)) create_directory(s);
	if (not_set_types) content_types = content_any_types, not_set_types = false;
	//if (not_set_types)file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt" }), not_set_types = false;
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
	if (uv_listen((uv_stream_t*)&_, max_conn, on_conn))return false;
	uv_run(uv_default_loop(), UV_RUN_DEFAULT); uv_loop_close(loop_); return true;
  }
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
	Conn* c = (Conn*)h->data;
	if (nread > 0) {
	  int failed = llhttp__internal_execute(&c->parser_, buf->base, buf->base + nread);
	  if (failed) { uv_close((uv_handle_t*)h, on_close); return; } Req& req = c->req_;
	  req = std::move(c->parser_.to_request());// printf("(%s,%s) ",req.url.c_str(),m2c(req.method));
	  if (!req.url(0)) return;
#if defined __linux__ || defined __APPLE__
	  if (c->parser_.http_minor != 0 && STR_KEY_EQ(get_header(req.headers, RES_Con), "close")) {
		uv_close((uv_handle_t*)h, on_close); return;
	  }
#endif
	  Res& res = c->res_; LOG_GER(m2c(req.method) << " |" << res.code << "| " << req.url);// c->_.data = &res;
	  // res.timer_.setTimeout([h] {
		 //uv_shutdown(&RES_SHUT_REQ, h, NULL); uv_close((uv_handle_t*)h, on_close);
	  // }, c->keep_milliseconds);// res.add_header(RES_Con, "Keep-Alive");
	  if (uv_now(c->loop_) - RES_last > 1000) {//uv_mutex_lock(&RES_MUTEX); uv_mutex_unlock(&RES_MUTEX);
		time(&RES_TIME_T); RES_last = uv_now(c->loop_);
#if defined(_MSC_VER) || defined(__MINGW32__)
		localtime_s(RES_NOW, &RES_TIME_T);
#else
		localtime_r(&RES_TIME_T, RES_NOW);
#endif
		RES_DATE_STR.resize(strftime(&RES_DATE_STR[0], 0x2f, RES_GMT, RES_NOW));
	  } //printf("<%s,%lld> ", req.params.c_str(), req.uuid);
	  Buf& s = c->buf_;
	  try {
		((App*)c->app_)->_call(req.method, req.url, req, res);
		//for (std::pair<const Buf, Buf>& kv : req.headers) std::cout << kv.first << RES_seperator << kv.second << RES_crlf;
		if (res.is_file > 0) {
		  if (res.is_file == 1) {
			c->set_status(res, res.code); s << RES_http_status << c->status_;
			res.is_file = 0; req.uuid = hackUrl(req.url.c_str());
			s << RES_CE << RES_seperator << RES_gzip << RES_crlf;
			if (RES_CACHE_TIME[req.uuid] > nowStamp()) { res.body << RES_CACHE_MENU[req.uuid]; goto _; }
			RES_CACHE_TIME[req.uuid] = nowStamp(CACHE_HTML_TIME_SECOND);
			s << RES_CT << RES_seperator << RES_Txt << RES_crlf; s << RES_date_tag << RES_DATE_STR << RES_crlf;
			std::ifstream inf(res.path_, std::ios::in | std::ios::binary);
			int l = 0; $:l = inf.read(c->readbuf, BUF_SIZE).gcount();
			if (l) { res.body << res.compress_str(c->readbuf, l); goto $; }
			inf.close(); s << RES_content_length_tag << res.body.size() << RES_crlf;
			s << RES_crlf << res.body; RES_CACHE_MENU[req.uuid] = res.body;
			c->write(s.data_, s.size()); s.reset(); res.zlib_cp_str.reset(); res.body.reset(); return;
		  }
		  Buf range = get_header(req.headers, RES_Range); bool is_range = false;
		  c->set_status(res, res.code); s << RES_http_status << c->status_;
		  if (range != "") {
			is_range = true; if (range[6] == '0')res.code = 206; if (range[6] != '0')res.code = 206;
		  }
		  for (std::pair<const Buf, Buf>& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
		  s << RES_AR << RES_seperator << RES_bytes << RES_crlf;
		  if (res.code == 206) {
			long i{ 0 }; range = range.substr(6).pop_back(); i = std::lexical_cast<long>(range); std::cout << ' ' << range << ',';
			s << RES_content_length_tag << std::to_string(res.file_size - i) << RES_crlf;
			s << RES_CR << RES_seperator << RES_bytes << ' ' << i << '-' << (res.file_size - 1) << '/' << res.file_size << RES_crlf;
			s << RES_crlf;
			res.is_file = 0; res.headers.clear();
			if (!c->write(s.data_, s.size())) { s.reset(); return; }; s.reset();
			if (res.__->ptr_ != nullptr) {
			  int r = res.__->read_chunk(0, res.file_size, [c](const char* s, size_t l, std::function<void()> d) {
				if (l > 0) { c->write(s, static_cast<int>(l)); if (d != nullptr) d(); } });
			}
			return;
		  }
		  s << RES_content_length_tag << std::to_string(res.file_size) << RES_crlf; if (res.code == 200)
			s << RES_Ca << RES_seperator << FILE_TIME << RES_crlf << RES_Xc << RES_seperator << RES_No << RES_crlf;
		  s << RES_crlf;
		  res.is_file = 0; res.headers.clear();
		  if (!c->write(s.data_, s.size())) { s.reset(); return; }; s.reset();
		  if (res.__->ptr_ != nullptr) {
			int r = res.__->read_chunk(0, res.file_size, [c](const char* s, size_t l, std::function<void()> d) {
			  if (l > 0) { c->write(s, static_cast<int>(l)); if (d != nullptr) d(); } });
		  }
		  return;
		}
		c->set_status(res, res.code); s << RES_http_status << c->status_;
	  } catch (const http_error& e) {
		s.reset(); c->set_status(res, e.i()); res.body = e.what(); s << RES_http_status << c->status_; goto _;
	  } catch (const std::exception& e) {
		s.reset(); c->set_status(res, 500); res.body = e.what(); s << RES_http_status << c->status_; goto _;
	  }
	  //if (STR_KEY_EQ(get_header(req.headers, RES_Ex), "100-continue") &&
	  // c->parser_.http_major == 1 && c->parser_.http_minor == 1)s << expect_100_continue;
	  for (std::pair<const Buf, Buf>& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
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
	_:
#if SHOW_SERVER_NAME
	  s << RES_server_tag << SERVER_NAME << RES_crlf;
#endif
	  if (!res.headers.count(RES_CT)) { s << RES_CT << RES_seperator << RES_Txt << RES_crlf; }
	  s << RES_content_length_tag << res.body.size() << RES_crlf;
	  s << RES_date_tag << RES_DATE_STR << RES_crlf;
	  s << RES_crlf << res.body; res.headers.clear(); res.code = 200; res.body.reset();
	  DEBUG("客户端：%lld %s \n", c->id, s.c_str());
	  c->write(s.data_, s.size()); s.reset();
	} else if (nread < 0) {
	  if (nread == UV_EOF || nread == UV_ECONNRESET) {
		DEBUG("1->%Id: %s : %s\n", nread, uv_err_name(nread), uv_strerror(nread));
		uv_close((uv_handle_t*)h, on_close);
	  } else {//UV_ENOBUFS
		DEBUG("2->%Id: %s : % s\n", nread, uv_err_name(nread), uv_strerror(nread));
		uv_close((uv_handle_t*)h, on_close);
	  }//if (!uv_is_active((uv_handle_t*)h))// uv_read_stop((uv_stream_t*)h);
	}//printf("%d\n", nread);uv_read_stop(h);
  }
  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested, uv_buf_t* b) { Conn* c = (Conn*)h->data; *b = c->rbuf; }
  void Tcp::on_close(uv_handle_t* h) {
	Conn* c = (Conn*)h->data; --((Tcp*)c->tcp_)->connection_num; DEBUG("{x%Id}\n", c->id); delete c;
  }
  void Tcp::on_conn(uv_stream_t* srv, int st) {
	Tcp* t = (Tcp*)srv->data; if (t->connection_num > t->max_conn) return;
	Conn* c = new Conn(t->keep_milliseconds, t->loop_);
	int $ = uv_tcp_init(t->loop_, &c->slot_); if ($) { delete c; return; }
	$ = uv_accept((uv_stream_t*)&t->_, (uv_stream_t*)&c->slot_);
	if ($) { uv_close((uv_handle_t*)&c->slot_, NULL); delete c; return; }
	c->app_ = t->app_; c->tcp_ = t;
	if (0 == uv_tcp_getpeername((uv_tcp_t*)&c->slot_, (sockaddr*)&t->addr_, &t->addr_len)) {
	  char name[128] = {}; if (!t->is_ipv6) {
		sockaddr_in addr = *((sockaddr_in*)&t->addr_);
		uv_inet_ntop(addr.sin_family, &addr.sin_addr, name, t->addr_len); c->req_.ip_addr = std::string(name, t->addr_len);
	  } else {
		sockaddr_in6 addr = *((sockaddr_in6*)&t->addr_);
		uv_inet_ntop(addr.sin6_family, &addr.sin6_addr, name, t->addr_len); c->req_.ip_addr = std::string(name, t->addr_len);
	  }
	}
#ifdef _WIN32
	c->id = c->slot_.socket;
#else
	c->id = uv__stream_fd(&c->slot_);
#endif
	c->set_keep_alive(c->id, 4, 2, 2); ++t->connection_num;
	uv_read_start((uv_stream_t*)&c->slot_, alloc_cb, read_cb);
  }
}
