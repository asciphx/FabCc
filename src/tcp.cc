#include "tcp.hh"
namespace fc {
  Tcp::Tcp(uv_loop_t* loop):opened(false) { loop_ = loop; }
  bool Tcp::init() {
	if (opened)return true; opened = true; if (!loop_)return false;
	if (uv_tcp_init(loop_, &_)) return false; _.data = this; return true;
  }
  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, on_exit); }
  void Tcp::test() {}
  Tcp::~Tcp() { exit(); }
  bool Tcp::setTcpNoDelay(bool enable) { return uv_tcp_nodelay(&_, enable ? 1 : 0) ? false : true; }
  bool Tcp::bind(const char* ip_addr, int port, bool is_ip4) {
	int $; if (is_ip4) {
	  struct sockaddr_in addr; $ = uv_ip4_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0);
	} else {
	  struct sockaddr_in6 addr; $ = uv_ip6_addr(ip_addr, port, &addr); if ($)return false;
	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0);
	} return $ ? true : false;
  }
  bool Tcp::listen(int backlog) { return uv_listen((uv_stream_t*)&_, backlog, on_connection) ? true : false; }
  bool Tcp::Start(const char* ip_addr, int port) {
	exit(); if (!init())return false; if (bind(ip_addr, port))return false;//bool is_ip4 = true
	if (listen())return false; uv_run(loop_, UV_RUN_DEFAULT); uv_loop_close(loop_); return false;
  }
  void Tcp::on_connection(uv_stream_t* server, int status) {
	Tcp* tcp = (Tcp*)server->data; Conn* co = new Conn(tcp->getParser());
	//co->tcp_ = tcp;//保存服务器的信息
	int $ = uv_tcp_init(tcp->loop_, co->ptr_); if ($) { delete co; return; }
	$ = uv_accept((uv_stream_t*)&tcp->_, (uv_stream_t*)co->ptr_);
	if ($) { uv_close((uv_handle_t*)co->ptr_, NULL); delete co; return; }
	sockaddr addr; sockaddr_in addrin; int addrlen = sizeof(addr); char sockname[17] = { 0 };
	if (0 == uv_tcp_getpeername((uv_tcp_t*)co->ptr_, &addr, &addrlen)) {
	  addrin = *((sockaddr_in*)&addr);
	  uv_ip4_name(&addrin, sockname, addrlen); co->id = addrin.sin_port;
	  co->request_.ip_addr = sockname; DEBUG(" %s:%d", sockname, ntohs(addrin.sin_port));
	}
	uv_tcp_keepalive(&tcp->_, 1, 4); uv_read_start((uv_stream_t*)co->ptr_, alloc_cb, read_cb);
  }
  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested_size, uv_buf_t* b) {
	Conn* wr = (Conn*)h->data; if (wr != nullptr) *b = wr->buf; else wr->buf.base = nullptr, wr->buf.len = 0;
  }
  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* b) {
	Conn* co = (Conn*)h->data; if (co == nullptr) return;
	if (nread > 0) {
	  co->buf = uv_buf_init(co->buf.base, nread);
	  int r = uv_write(&co->_, h, &co->buf, 1, write_cb);
	  if (r) { DEBUG("uv_write error: %s", uv_strerror(r)); return; }
	  //DEBUG("客户端：%d %s \n", wr->id, b->base);
	  bool failed = llhttp__internal_execute(co->parser_, b->base, b->base + nread);
	  if (!failed) {
		Req* req = &co->request_;
		req->method = static_cast<HTTP>(co->parser_->method);
		req->url = std::move(co->parser_->url);
		req->raw_url = std::move(co->parser_->raw_url);
		req->body = std::move(co->parser_->body);
		req->headers = std::move(co->parser_->headers);
		for (auto l : req->headers) {
		  DEBUG("%s: %s, ", l.first.c_str(), l.second.c_str());
		}
		req = nullptr;
		printf("\n[%s]", co->request_.raw_url.c_str());
	  }
	  //r = on_socket_read(nread, b);
	  //if(r == -4097) set_read_done();
	  //else if(r < 0) DEBUG("%p:%p read tcp: %s\n", wr, tcp, uv_err_name(r));
	} else if (0 == nread) { free(b->base); } else if (nread < 0) {
	  Tcp* server = (Tcp*)(h)-((size_t) & reinterpret_cast<char const volatile&>(((Tcp*)0)->_));
	  server->test();// TYPE_GET(Tcp, h, _);
	  // int r = on_socket_read(nread, buf);
	  // if (r == UV_EOF) set_read_done();
	  // bool done = wr->is_read_done();
	  // if (r < 0 || done) on_read_end(r);
	  if (nread == UV_EOF) {
		DEBUG("客户id(%d)断开\n", co->id);
	  } else if (nread == UV_ECONNRESET) {
		DEBUG("客户id(%d)异常\n", co->id);
	  } else {
		DEBUG("%p:%p name: %s err: %s\n", co, tcp, uv_err_name(nread), uv_strerror(nread));
	  }
	  //if (uv_is_active((uv_handle_t*)wr->ptr_)) uv_read_stop((uv_stream_t*)wr->ptr_);
	  uv_close((uv_handle_t*)co->ptr_, on_close);
	}
  }
  void Tcp::write_cb(uv_write_t* co, int status) {
	if (status < 0) { DEBUG("写数据错误 %s\n", uv_strerror(status)); }
	//Conn* wr; wr = (Conn*)co; free(wr->buff.base); wr->buff.base = nullptr; wr->buff.len = 0;
  }
  void Tcp::on_exit(uv_handle_t* h) {}
  void Tcp::on_close(uv_handle_t* h) {
	Conn* wr = (Conn*)h->data; DEBUG("客户端：%d 关闭！ \n", wr->id); delete wr;
  }
}
