#include <socket.hh>
#include <conn.hh>
#include <tcp.hh>
namespace fc {
  Socket::Socket(unsigned short milliseconds) noexcept{ }
  bool Socket::write(const char* c, int i) {
	if (!c || !i) { return true; }
	const char* e = c + i; int l = ::send(this->id, c, e - c, 0);
	if (l > 0) c += l; while (c != e) {
	  if ((l < 0 && errno != EAGAIN) || l == 0) return false;
	  l = ::send(this->id, c, int(e - c), 0); if (l > 0) c += l;
	} return true;
  };
  int Socket::read(char* buf, int size) {
	int count = ::recv(this->id, buf, size, 0);
	while (count <= 0) {
	  if ((count < 0 && errno != EAGAIN) || count == 0)
		return int(0);
	  count = ::recv(this->id, buf, size, 0);
	}
	return count;
  };
  int Socket::shut(socket_type fd, send_type type) { return ::shutdown(fd, type); }
  int Socket::shut(send_type type) { return ::shutdown(this->id, type); }
  int Socket::close_fd(socket_type fd) {
#if defined _WIN32
	return ::closesocket(fd);
#else
	return ::close(fd);
#endif
  }
  //uv__socket_sockopt, uv_poll_init, uv_poll_init_socket
  int Socket::set_keep_alive(socket_type& fd, int idle, int intvl, unsigned char probes) {
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
	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&idle, sizeof(idle)); setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&intvl, sizeof(intvl));
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&probes, sizeof(int)); setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &RES_RCV, sizeof(RES_RCV));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &RES_SED, sizeof(RES_SED));
#endif
	return 0;
  }
	bool Socket::is_open() { return true; }
	void Socket::close() {
	  //Conn* c = (Conn*)slot_.data; --((Tcp*)c->tcp_)->connection_num;
	  //((Tcp*)c->tcp_)->$.erase(static_cast<unsigned int>(c->id)); delete c;
	}
}
