#include "hh/conn.hh"
namespace fc {//If it exceeds 6(k_a) seconds by default, the established connection will be closed but writing has not yet started
#if __cplusplus < _cpp20_date
  int Conn::read(char* buf, int max_size) {
    int count = read_impl(buf, max_size); int64_t t = 0;
    while (count < 0) {
#ifndef _WIN32
      if (errno != EAGAIN) { return 0; }
#else
      if (errno != EINPROGRESS && errno != EINVAL && errno != ENOENT) { return 0; }
#endif // !_WIN32
      rpg->_.operator()(); t = time(NULL) - rpg->hrt;
      if (is_idle && t) {
        if (t > k_a) {
#ifdef _WIN32
          ::setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, (const char*)&RESling, sizeof(linger));
#endif
          return 0;
        }
        u16 n = ++rpg->idx; ROG* fib = rpg; timer.add_s(k_a + 1, [n, fib] { if (fib->idx == n && fib->_) { fib->_.operator()(); } });
      }
      count = read_impl(buf, max_size);
    }
    return count;
  };//Alternatively, you can control the maximum number of connections that can be established per IP, which has not yet been implemented
  bool Conn::write(const char* buf, int size) {
    const char* end = buf + size; is_idle = false;
    int count = write_impl(buf, size); if (count > 0) buf += count;
    while (buf != end) {
#ifndef _WIN32
      if (count == 0 || count < 0 && errno != EAGAIN) { return false; }
#else
      if (count == 0 || count < 0 && (errno != EINVAL && errno != EINPROGRESS)) { return false; }
#endif // !_WIN32
      rpg->_.operator()();
      if ((count = write_impl(buf, int(end - buf))) > 0) buf += count;
    } time(&rpg->hrt);
    return is_idle = true;
  };
  bool Conn::writen(const char* buf, int size) {
    const char* end = buf + size; is_idle = false;
    int count = write_impl(buf, size); if (count > 0) buf += count;
    while (buf != end) {
#ifndef _WIN32
      if (count == 0 || count < 0 && (errno == EPIPE || errno != EAGAIN)) { return false; }
#else
      if (count == 0 || count < 0 && (errno == EPIPE || (errno != EINVAL && errno != EINPROGRESS))) { return false; }
#endif // !_WIN32
      rpg->_.operator()();
      if ((count = write_impl(buf, int(end - buf))) > 0) buf += count;
    }
    return is_idle = true;
  };
#endif
  int Conn::shut(socket_type fd, sd_type type) { return ::shutdown(fd, type); }
  int Conn::shut(sd_type type) { return ::shutdown(this->socket_fd, type); }
  void Conn::epoll_mod(socket_type flags) {
#if __linux__ || _WIN32
    epoll_event e{ static_cast<uint32_t>(flags) }; e.data.ptr = rpg;
    if (-1 == ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &e) && errno != EEXIST) std::cout << "epoll_ctl error: " << strerror(errno) << std::endl;
#elif __APPLE__
    struct kevent e; e.udata = rpg; EV_SET(&e, socket_fd, flags, 2, 0, 0, NULL); kevent(epoll_fd, &e, 1, NULL, 0, NULL);
#endif
  }
}