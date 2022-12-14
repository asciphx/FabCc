#include <conn.hh>
#include <tcp.hh>
namespace fc {
  void Conn::epoll_add(socket_type fd, int flags) { reactor->epoll_add(fd, flags, fiber_id); }
  void Conn::epoll_mod(socket_type fd, int flags) { reactor->epoll_mod(fd, flags); }
  void Conn::defer_fiber_resume(socket_type fiber_id) { reactor->defered_resume.push_back(fiber_id); }
  void Conn::reassign_fd_to_this_fiber(socket_type fd) {
	reactor->reassign_fd_to_fiber(fd, this->fiber_id);
  }
  void Conn::defer(const std::function<void()>&fun) { reactor->defered_functions.push_back(fun); }
  int Conn::read(char* buf, int max_size) {
	int count = read_impl(buf, max_size);
	while (count <= 0) {
	  if ((count < 0 && errno != EAGAIN) || count == 0) return int(0);
	  sink = sink.yield();
	  count = read_impl(buf, max_size);
	}
	return count;
  };
  bool Conn::write(const char* buf, int size) {
	//if (!buf || !size) { sink = sink.yield(); return true; }
	const char* end = buf + size;
	int count = write_impl(buf, int(end - buf));
	if (count > 0) buf += count;
	while (buf != end) {
	  if ((count < 0 && errno != EAGAIN) || count == 0) return false;
	  sink = sink.yield();
	  count = write_impl(buf, int(end - buf));
	  if (count > 0) buf += count;
	}
	return true;
  };
  int Conn::shut(socket_type fd, sd_type type) { return ::shutdown(fd, type); }
  int Conn::shut(sd_type type) { return ::shutdown(this->socket_fd, type); }
  int Conn::set_keep_alive(socket_type fd, int idle, int intvl, unsigned char probes) {
#ifdef WIN32
	struct tcp_keepalive in_kavars; memset(&in_kavars, 0, sizeof(in_kavars)); unsigned long in_len = sizeof(struct tcp_keepalive);
	struct tcp_keepalive out_kavars; memset(&out_kavars, 0, sizeof(out_kavars)); unsigned long out_len = sizeof(struct tcp_keepalive);
	in_kavars.onoff = probes; in_kavars.keepalivetime = idle * 1000; in_kavars.keepaliveinterval = intvl * 1000; DWORD ulBytesReturn;
	if (WSAIoctl(fd, SIO_KEEPALIVE_VALS, (LPVOID)&in_kavars, in_len, (LPVOID)&out_kavars, out_len, &ulBytesReturn, NULL, NULL) == SOCKET_ERROR) {
	  DEBUG("WSAIoctl() SIO_KEEPALIVE_VALS error. [%d]", WSAGetLastError()); return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&RES_RCV, sizeof(RES_RCV));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&RES_SED, sizeof(RES_SED));
#else
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &RES_KEEP_Ai, sizeof(RES_KEEP_Ai)) != 0) return -1;
	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&idle, sizeof(idle)); setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&intvl, sizeof(intvl));
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&probes, sizeof(int)); setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &RES_RCV, sizeof(RES_RCV));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &RES_SED, sizeof(RES_SED));
#endif
	return 0;
  }
}
