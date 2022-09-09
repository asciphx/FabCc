#include <conn.hh>

namespace fc {
  Conn::Conn(unsigned short milliseconds, uv_loop_t* l)
	 noexcept: loop_(l), buf_(0x3ff), keep_milliseconds(milliseconds) {
	fs_.data = this; slot_.data = this; rbuf = uv_buf_init((char*)malloc(BUF_SIZE), BUF_SIZE);
	sink_ = [this](const char* data, size_t size, std::function<void()> done) {
#ifdef _WIN32
	  wbuf.base = const_cast<char*>(data);
	  wbuf.len = static_cast<decltype(wbuf.len)>(size);
	  if (wbuf.len > 0) {
		uv_write(&_, (uv_stream_t*)&slot_, &wbuf, 1, NULL);
		if (done != nullptr) done();
	  }
#else
	  buf_ << std::string_view(data, size);
	  if (done != nullptr) done();
	  wbuf.base = buf_.data_; wbuf.len = buf_.size();
	  uv_write(&_, (uv_stream_t*)&slot_, &wbuf, 1, NULL); buf_.reset();
#endif // _WIN32
	};
  }
  Conn::~Conn() {
	free(rbuf.base); rbuf.base = nullptr; app_ = nullptr; loop_ = nullptr;
	tcp_ = nullptr; fs_.data = nullptr; sink_ = nullptr;
  }
  bool Conn::write(const char* c, int i) {
	if (!c || !i) { return true; }
	const char* e = c + i; int l = ::send(id, c, e - c, 0);
	if (l > 0) c += l; while (c != e) {
	  if ((l < 0 && errno != EAGAIN) || l == 0) return false;
	  l = ::send(id, c, int(e - c), 0); if (l > 0) c += l;
	} return true;
  };
  //  int Conn::shut(socket_type fd, sd_type type) {
  //#if defined _WIN32
  //	return ::shutdown(fd, type);//SD_RECEIVE，SD_SEND，SD_BOTH
  //#else
  //	return ::shutdown(fd, type);
  //#endif
  //  }
  //  int Conn::close_fd(socket_type fd) {
  //#if defined _WIN32
  //	closesocket(fd); WSACleanup(); return 1;
  //#else
  //	return close(fd);
  //#endif
  //  }
  //uv__socket_sockopt, uv_poll_init, uv_poll_init_socket
  int Conn::set_keep_alive(socket_type& fd, int idle, int intvl, unsigned char probes) {
#ifdef WIN32
	//if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&idle, sizeof idle))return -1;
	//if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&intvl, sizeof intvl))return -1;
	//if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&probes, sizeof(int)))return -1;
	//if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&RES_KEEP_Alive, sizeof RES_KEEP_Alive) != 0) return -1;
	struct tcp_keepalive in_kavars; memset(&in_kavars, 0, sizeof(in_kavars)); unsigned long in_len = sizeof(struct tcp_keepalive);
	struct tcp_keepalive out_kavars; memset(&out_kavars, 0, sizeof(out_kavars)); unsigned long out_len = sizeof(struct tcp_keepalive);
	in_kavars.onoff = probes; in_kavars.keepalivetime = idle * 1000; in_kavars.keepaliveinterval = intvl * 1000; DWORD ulBytesReturn;
	if (WSAIoctl(fd, SIO_KEEPALIVE_VALS, (LPVOID)&in_kavars, in_len, (LPVOID)&out_kavars, out_len, &ulBytesReturn, NULL, NULL) == SOCKET_ERROR) {
	  DEBUG("WSAIoctl() SIO_KEEPALIVE_VALS error. [%d]", WSAGetLastError()); return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&RES_RCV, sizeof(RES_RCV));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&RES_SED, sizeof(RES_SED));
#else
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &RES_KEEP_Alive, sizeof(RES_KEEP_Alive)) != 0) return -1;
	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&idle, sizeof(idle));
	setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&intvl, sizeof(intvl));//second
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&probes, sizeof(int));
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &RES_RCV, sizeof(RES_RCV));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &RES_SED, sizeof(RES_SED));
#endif
	return 0;
  }
  void Conn::set_status(Res& res, uint16_t status) {
	res.code = status;
	switch (status) {
	case 200:status_ = "200 OK\r\n"; break;
	case 201:status_ = "201 Created\r\n"; break;
	case 202:status_ = "202 Accepted\r\n"; break;
	case 203:status_ = "203 Non-Authoritative Information\r\n"; break;
	case 204:status_ = "204 No Content\r\n"; break;

	case 301:status_ = "301 Moved Permanently\r\n"; break;
	case 302:status_ = "302 Found\r\n"; break;
	case 303:status_ = "303 See Other\r\n"; break;
	case 304:status_ = "304 Not Modified\r\n"; break;
	case 307:status_ = "307 Temporary redirect\r\n"; break;

	case 400:status_ = "400 Bad Request\r\n"; break;
	case 401:status_ = "401 Unauthorized\r\n"; break;
	case 402:status_ = "402 Payment Required\r\n"; break;
	case 403:status_ = "403 Forbidden\r\n"; break;
	case 405:status_ = "405 HTTP verb used to access this page is not allowed\r\n"; break;
	case 406:status_ = "406 Browser does not accept the MIME type of the requested page\r\n"; break;
	case 409:status_ = "409 Conflict\r\n"; break;

	case 500:status_ = "500 Internal Server Error\r\n"; break;
	case 501:status_ = "501 Not Implemented\r\n"; break;
	case 502:status_ = "502 Bad Gateway\r\n"; break;
	case 503:status_ = "503 Service Unavailable\r\n"; break;
	case 510:status_ = "510 Not Extended\r\n"; break;
	default: status_ = "404 Not Found\r\n"; res.code = 404;
	}
  }
}
