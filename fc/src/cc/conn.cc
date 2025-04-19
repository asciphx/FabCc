#include "hh/conn.hh"
namespace fc {//If it exceeds 5 seconds by frist request, the established connection will be closed but writing has not yet started
  _CTX_TASK(int) Conn::read(char* buf, int max_size) {
    int count = read_impl(buf, max_size); int64_t t = 0;
    while (count < 0) {
#ifndef _WIN32
      if (errno != EAGAIN) { _CTX_back(0) }
#else
      if (errno != EINPROGRESS && errno != EINVAL && errno != ENOENT) { _CTX_back(0) }
#endif // !_WIN32
#if __cplusplus < _cpp20_date
      rpg->_.operator()();
#else
      co_await std::suspend_always{};
#endif
      t = time(NULL) - rpg->hrt;
      if (is_idle && t) {
        if (t > k_a) {
#ifdef _WIN32
          ::setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, (const char*)&RESling, sizeof(linger));
#endif
          _CTX_back(0)
        }
      }
      count = read_impl(buf, max_size);
    }
    _CTX_back(count)
  };//Alternatively, you can control the maximum number of connections that can be established per IP, which has not yet been implemented
  _CTX_TASK(bool) Conn::write(const char* buf, int size) {
    is_idle = false; int count = write_impl(buf, size); if (count > 0) buf += count, size -= count;
    while (size != 0) {
#ifndef _WIN32
      if (count == 0 || count < 0 && errno != EAGAIN) { _CTX_back(false) }
#else
      if (count == 0 || count < 0 && (errno != EINPROGRESS && errno != EINVAL)) { _CTX_back(false) }
#endif // !_WIN32
#if __cplusplus < _cpp20_date
      rpg->_.operator()();
#else
      co_await std::suspend_always{};
#endif
      if ((count = write_impl(buf, size)) > 0) { buf += count, size -= count; }
    } time(&rpg->hrt);
    timer.cancel(rpg->t_id); ROG* fib = rpg; rpg->t_id = timer.add_s(k_a + 2, [fib] { if (fib->_) { fib->_.operator()(); } });
    _CTX_back(is_idle = true)
  };
  //It is also possible to add flow control to limit the transmission speed
  _CTX_TASK(bool) Conn::send(const char* buf, int size) {
    is_idle = false; int count = write_impl(buf, size); if (count > 0) buf += count, size -= count;
    while (size != 0) {
#ifndef _WIN32
      if (count == 0 || count < 0 && errno != EAGAIN) { _CTX_back(false) }
#else
      if (count == 0 || count < 0 && (errno != EINPROGRESS && errno != EINVAL)) { _CTX_back(false) }
#endif // !_WIN32
#if __cplusplus < _cpp20_date
      rpg->_.operator()();
#else
      co_await std::suspend_always{};
#endif
      if ((count = write_impl(buf, size)) > 0) { buf += count, size -= count; }
    }
    _CTX_back(is_idle = true)
  };
  int Conn::shut(socket_type fd, sd_type type) { return ::shutdown(fd, type); }
  int Conn::shut(sd_type type) { return ::shutdown(this->socket_fd, type); }
  void Conn::epoll_fix(socket_type flags, int action) {
#if __linux__ || _WIN32
    epoll_event e{ static_cast<uint32_t>(flags) }; e.data.ptr = rpg;
    if (-1 == ::epoll_ctl(epoll_fd, action, socket_fd, &e) && errno != EEXIST) std::cout << "epoll_ctl err: " << strerror(errno) << std::endl;
#elif __APPLE__
    struct kevent e; e.udata = rpg; EV_SET(&e, socket_fd, flags, action, 0, 0, NULL); kevent(epoll_fd, &e, 1, NULL, 0, NULL);
#endif
  }
}