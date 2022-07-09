#include "tcp.hh"
namespace fc {
  Tcp::Tcp(uv_loop_t* loop):opened(false), loop_(loop), addr_len(16) {}
  void Tcp::thread(unsigned char n) { num = n; }
  bool Tcp::init() {
	if (opened)return true; opened = true; if (!loop_)return false;
	if (uv_tcp_init(loop_, &_)) return false; _.data = this; return true;
  }
  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, on_exit); }
  Tcp::~Tcp() { exit(); }
  bool Tcp::setTcpNoDelay(bool enable) { return uv_tcp_nodelay(&_, enable ? 1 : 0) ? false : true; }
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
  bool Tcp::Start(const char* ip_addr, int port) {
	exit(); if (!port)port = port_; if (!init())return false; if (bind(ip_addr, port))return false;
	if (listen())return false; uv_run(loop_, UV_RUN_DEFAULT); uv_loop_close(loop_); return false;
  }
  void Tcp::set_status(Conn* co, uint16_t status) {
	co->res_.code = status;
	switch (status) {
	case 200:co->status_ = "200 OK\r\n"; break;
	case 201:co->status_ = "201 Created\r\n"; break;
	case 202:co->status_ = "202 Accepted\r\n"; break;
	case 203:co->status_ = "203 Non-Authoritative Information\r\n"; break;
	case 204:co->status_ = "204 No Content\r\n"; break;
	case 301:co->status_ = "301 Moved Permanently\r\n"; break;
	case 302:co->status_ = "302 Found\r\n"; break;
	case 303:co->status_ = "303 See Other\r\n"; break;
	case 304:co->status_ = "304 Not Modified\r\n"; break;
	case 307:co->status_ = "307 Temporary redirect\r\n"; break;
	case 400:co->status_ = "400 Bad Request\r\n"; co->res_.body = co->status_; break;
	case 401:co->status_ = "401 Unauthorized\r\n"; co->res_.body = co->status_; break;
	case 402:co->status_ = "402 Payment Required\r\n"; co->res_.body = co->status_; break;
	case 403:co->status_ = "403 Forbidden\r\n"; co->res_.body = co->status_; break;
	case 405:co->status_ = "405 HTTP verb used to access this page is not allowed\r\n"; co->res_.body = co->status_; break;
	case 406:co->status_ = "406 Browser does not accept the MIME type of the requested page\r\n"; co->res_.body = co->status_; break;
	case 409:co->status_ = "409 Conflict\r\n"; co->res_.body = co->status_; break;
	case 500:co->status_ = "500 Internal Server Error\r\n"; break;
	case 501:co->status_ = "501 Not Implemented\r\n"; co->res_.body = co->status_; break;
	case 502:co->status_ = "502 Bad Gateway\r\n"; co->res_.body = co->status_; break;
	case 503:co->status_ = "503 Service Unavailable\r\n"; co->res_.body = co->status_; break;
	default:co->status_ = "404 Not Found\r\n"; co->res_.body = co->status_; co->res_.code = 404; break;
	}
  }
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
	Conn* co = (Conn*)h->data; if (co == nullptr) return;
	if (nread > 0) {
	  bool failed = llhttp__internal_execute(&co->parser_, buf->base, buf->base + nread);
	  if (failed) { uv_close((uv_handle_t*)h, on_close); return; }
	  // if(co->parser_.keep_alive){
		 //printf("keep-alive!");
	  // }
	  Req* req = &co->req_; req->method = static_cast<HTTP>(co->parser_.method);
	  req->url = std::move(co->parser_.url); req->raw_url = std::move(co->parser_.raw_url);
	  req->body = std::move(co->parser_.body); req->headers = std::move(co->parser_.headers);

	  Buffer b; //printf("[%s]", co->req_.raw_url.c_str());
	  if (co->parser_.http_major == 1 && co->parser_.http_minor == 1 &&
		get_header(co->parser_.headers, RES_Ex) == "100-continue") b << expect_100_continue;//co->buf_ += expect_100_continue;
	  co->res_.body = "<html>hello world!</html>";
	  set_status(co, 200);
	  b << Res_http_status << co->status_;
	  co->buf_ += Res_http_status; co->buf_ += co->status_;
#ifdef AccessControlAllowCredentials
	  b << RES_AcC << AccessControlAllowCredentials << Res_crlf;
	  //co->buf_ += RES_AcC; co->buf_ += AccessControlAllowCredentials; co->buf_ += Res_crlf;
#endif
#ifdef AccessControlAllowHeaders
	  b << RES_AcH << AccessControlAllowHeaders << Res_crlf;
	  //co->buf_ += RES_AcH; co->buf_ += AccessControlAllowHeaders; co->buf_ += Res_crlf;
#endif
#ifdef AccessControlAllowMethods
	  b << RES_AcM << AccessControlAllowMethods << Res_crlf;
	  //co->buf_ += RES_AcM; co->buf_ += AccessControlAllowMethods; co->buf_ += Res_crlf;
#endif
#ifdef AccessControlAllowOrigin
	  b << RES_AcO << AccessControlAllowOrigin << Res_crlf;
	  //co->buf_ += RES_AcO; co->buf_ += AccessControlAllowOrigin; co->buf_ += Res_crlf;
#endif
	  if (!co->res_.headers.count(RES_CT)) {
		b << RES_CT << Res_seperator << RES_Txt << Res_crlf;
		//co->buf_ += RES_CT; co->buf_ += Res_seperator; co->buf_ += RES_Txt; co->buf_ += Res_crlf;
	  }
	  b << Res_content_length_tag << co->res_.body.size() << Res_crlf;
	  //co->buf_ += Res_content_length_tag;
	  //co->buf_ += std::to_string(co->res_.body.size()); co->buf_ += Res_crlf;
#if SHOW_SERVER_NAME
	  b << Res_server_tag << SERVER_NAME << Res_crlf;
	  //co->buf_ += Res_server_tag; co->buf_ += SERVER_NAME; co->buf_ += Res_crlf;
#endif
	  b << Res_crlf;
	  //co->buf_ += Res_crlf;
	  co->buf_ = b.to_string_view();
	  co->buf_ += std::move(co->res_.body);
	  //DEBUG("客户端：%d %s \n", co->id, co->buf_.c_str());
	  uv_buf_t wbuf = uv_buf_init(co->buf_.data(), co->buf_.size());
	  int r = uv_write(&co->_, h, &wbuf, 1, NULL);
	  if (r) { DEBUG("uv_write error: %s\n", uv_strerror(r)); return; }
	  //co->buf_.clear();
	} else if (nread < 0) {
	  // if (nread == UV_EOF) {
		 //DEBUG("客户id(%d)断开\n", co->id);
	  // } else if (nread == UV_ECONNRESET) {
		 //DEBUG("客户id(%d)异常\n", co->id);
	  // } else {
		 //DEBUG("name: %s err: %s\n", uv_err_name(nread), uv_strerror(nread));
	  // }
	  uv_close((uv_handle_t*)co->ptr_, on_close);
	  //if (!uv_is_active((uv_handle_t*)co->ptr_))// uv_read_stop((uv_stream_t*)co->ptr_);
	}
  }
  void Tcp::write_cb(uv_write_t* wr, int status) {
	Conn* co = (Conn*)wr; if (status) { uv_close((uv_handle_t*)co->ptr_, on_close); return; };
  }
  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested_size, uv_buf_t* b) {
	Conn* c = (Conn*)h->data; if (c != nullptr) *b = c->rbuf;
  }
  void Tcp::on_exit(uv_handle_t* h) {}
  void Tcp::on_close(uv_handle_t* h) {
	Conn* c = (Conn*)h->data; DEBUG("客户端：%d 关闭！ \n", c->id); delete c;
  }
  void Tcp::on_connection(uv_stream_t* srv, int status) {
	Tcp* tcp = (Tcp*)srv->data; Conn* co = new Conn();
	int $ = uv_tcp_init(tcp->loop_, co->ptr_); if ($) { delete co; return; }
	$ = uv_accept((uv_stream_t*)&tcp->_, (uv_stream_t*)co->ptr_);
	if ($) { uv_close((uv_handle_t*)co->ptr_, NULL); delete co; return; }
	co->req_.ip_addr.resize(tcp->addr_len);
	if (0 == uv_tcp_getpeername((uv_tcp_t*)co->ptr_, (sockaddr*)&tcp->addr_, &tcp->addr_len)) {
	  if (!tcp->is_ipv6) {
		sockaddr_in addrin = *((sockaddr_in*)&tcp->addr_);
		uv_ip4_name(&addrin, (char*)co->req_.ip_addr.data(), tcp->addr_len); co->id = addrin.sin_port;
	  } else {
		sockaddr_in6 addrin = *((sockaddr_in6*)&tcp->addr_);
		uv_ip6_name(&addrin, (char*)co->req_.ip_addr.data(), tcp->addr_len); co->id = addrin.sin6_port;

	  }
	  DEBUG(" %s:%d\n", co->req_.ip_addr.c_str(), ntohs(co->id));
	}
	uv_tcp_keepalive(&tcp->_, 1, 4); uv_read_start((uv_stream_t*)co->ptr_, alloc_cb, read_cb);
  }
}
