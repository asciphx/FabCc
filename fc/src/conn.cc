#include <conn.hh>
#include <tcp.hh>
namespace fc {
  void Conn::epoll_mod(socket_type fd, int flags) { reactor->epoll_mod(fd, flags); }
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
  void Conn::async(std::function<void()>&& func){
    func();
  };
  int Conn::shut(socket_type fd, sd_type type) { return ::shutdown(fd, type); }
  int Conn::shut(sd_type type) { return ::shutdown(this->socket_fd, type); }
  int Conn::set_keep_alive(socket_type fd, int idle, int intvl, unsigned char probes) {
#ifdef WIN32
    RES_in_kavars.onoff = probes; RES_in_kavars.keepalivetime = idle * 1000; RES_in_kavars.keepaliveinterval = intvl * 1000;
    if (WSAIoctl(fd, SIO_KEEPALIVE_VALS, (LPVOID)&RES_in_kavars, RES_l_k, (LPVOID)&RES_out_kavars, RES_l_k, &RES_uBR, NULL, NULL) == SOCKET_ERROR) {
      DEBUG("WSAIoctl() SIO_KEEPALIVE_VALS error. [%d]", WSAGetLastError()); return -1;
    }
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&RES_RCV, sizeof(RES_RCV));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&RES_SED, sizeof(RES_SED));
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&RES_nodelay, sizeof(int));
#else
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &RES_KEEP_Ai, sizeof(RES_KEEP_Ai)) != 0) return -1;
    setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&idle, sizeof(idle)); setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&intvl, sizeof(intvl));
    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&probes, sizeof(int)); setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &RES_RCV, sizeof(RES_RCV));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &RES_SED, sizeof(RES_SED));
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &RES_nodelay, sizeof(int));
#endif
    return 0;
  }
}
