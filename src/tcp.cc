#include "tcp.hh"
namespace fc {
  Tcp::Tcp(uv_loop_t* loop):opened(false), loop_(loop), addr_len(16) {}
  bool Tcp::init() {
	if (opened)return true; opened = true; if (!loop_)return false;
	if (uv_tcp_init(loop_, &_)) return false; _.data = this; return true;
  }
  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, on_exit); }
  void Tcp::test() {}
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
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
	Conn* co = (Conn*)h->data; if (co == nullptr) return;
	if (nread > 0) {
	  co->buf = uv_buf_init(co->buf.base, nread);
	  //DEBUG("客户端：%d %s \n", co->id, buf->base);
	  int r = uv_write(&co->_, h, &co->buf, 1, write_cb);
	  if (r) { DEBUG("uv_write error: %s\n", uv_strerror(r)); return; }
	  bool failed = llhttp__internal_execute(co->parser_, buf->base, buf->base + nread);
	  if (!failed) {
		Req* req = &co->request_;
		req->method = static_cast<HTTP>(co->parser_->method);
		req->url = std::move(co->parser_->url);
		req->raw_url = std::move(co->parser_->raw_url);
		req->body = std::move(co->parser_->body);
		req->headers = std::move(co->parser_->headers);
		for (auto l : req->headers) {
		  DEBUG("%s: %s, ", l.first.c_str(), l.second.c_str());
		} DEBUG("\n");
		req = nullptr;
		printf("[%s]", co->request_.raw_url.c_str());
	  }
	  //r = on_socket_read(nread, buf);
	  //if(r == -4097) set_read_done();
	  //else if(r < 0) DEBUG("%p:%p read tcp: %s\n", co, tcp, uv_err_name(r));
	} else if (0 == nread) { free(buf->base); } else if (nread < 0) {
	  Tcp* srv = (Tcp*)(h)-((size_t) & reinterpret_cast<char const volatile&>(((Tcp*)0)->_));
	  srv->test();// TYPE_GET(Tcp, h, _);
	  // int r = on_socket_read(nread, buf);
	  // if (r == UV_EOF) set_read_done();
	  // bool done = co->is_read_done();
	  // if (r < 0 || done) on_read_end(r);
	  if (nread == UV_EOF) {
		DEBUG("客户id(%d)断开\n", co->id);
	  } else if (nread == UV_ECONNRESET) {
		DEBUG("客户id(%d)异常\n", co->id);
	  } else {
		DEBUG("name: %s err: %s\n", uv_err_name(nread), uv_strerror(nread));
	  }
	  //if (uv_is_active((uv_handle_t*)co->ptr_)) uv_read_stop((uv_stream_t*)co->ptr_);
	  uv_close((uv_handle_t*)co->ptr_, on_close);
	}
  }
  void Tcp::write_cb(uv_write_t* co, int status) {
	if (status < 0) { DEBUG("写数据错误 %s\n", uv_strerror(status)); }
	//Conn* c; c = (Conn*)co; free(c->buff.base); c->buff.base = nullptr; c->buff.len = 0;
  }
  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested_size, uv_buf_t* b) {
	Conn* c = (Conn*)h->data; if (c != nullptr) *b = c->buf; else c->buf.base = nullptr, c->buf.len = 0;
  }
  void Tcp::on_exit(uv_handle_t* h) {}
  void Tcp::on_close(uv_handle_t* h) {
	Conn* c = (Conn*)h->data; DEBUG("客户端：%d 关闭！ \n", c->id); delete c;
  }
  void Tcp::on_connection(uv_stream_t* srv, int status) {
	Tcp* tcp = (Tcp*)srv->data; Conn* co = new Conn(tcp->getParser());
	int $ = uv_tcp_init(tcp->loop_, co->ptr_); if ($) { delete co; return; }
	$ = uv_accept((uv_stream_t*)&tcp->_, (uv_stream_t*)co->ptr_);
	if ($) { uv_close((uv_handle_t*)co->ptr_, NULL); delete co; return; }
	co->request_.ip_addr.resize(tcp->addr_len);
	if (0 == uv_tcp_getpeername((uv_tcp_t*)co->ptr_, (sockaddr*)&tcp->addr_, &tcp->addr_len)) {
	  if (!tcp->is_ipv6) {
		sockaddr_in addrin = *((sockaddr_in*)&tcp->addr_);
		uv_ip4_name(&addrin, (char*)co->request_.ip_addr.data(), tcp->addr_len); co->id = addrin.sin_port;
	  } else {
		sockaddr_in6 addrin = *((sockaddr_in6*)&tcp->addr_);
		uv_ip6_name(&addrin, (char*)co->request_.ip_addr.data(), tcp->addr_len); co->id = addrin.sin6_port;

	  }
	  DEBUG(" %s:%d\n", co->request_.ip_addr.c_str(), ntohs(co->id));
	}
	uv_tcp_keepalive(&tcp->_, 1, 4); uv_read_start((uv_stream_t*)co->ptr_, alloc_cb, read_cb);
  }
}
