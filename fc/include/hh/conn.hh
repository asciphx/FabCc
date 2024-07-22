#ifndef CONN_HH
#define CONN_HH
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <deque>
#include <thread>
#include <vector>
#if __linux__
#include <sys/epoll.h>
#elif __APPLE__
#include <sys/event.h>
#endif
#if defined __linux__ || defined __APPLE__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifndef _WIN32
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#else
#include <WS2tcpip.h>
#include <WinSock2.h>
#include "h/wepoll.h"
#include <mstcpip.h>
#endif
#include <chrono>
#include <atomic>
#include "h/config.h"
#include "tp/ctx.hh"
#include "hpp/http_top_header_builder.hpp"
#ifndef _OPENSSL
#define _OPENSSL 0
#endif
#if __cplusplus < _cpp20_date
#define _CTX_FUNC void(Conn&,void*)
#define _CTX_TASK(_) _
#define _CTX_back(_) return _;
#define _CTX_return return;
#define _ctx -> void
//co_return
#define _CTX_idx
#define _CTX_idex
#else
#define _CTX_FUNC fc::Task<void>(socket_type,sockaddr,int,fc::timer&,ROG*,epoll_handle_t,void*,int&,Reactor*)
#define _CTX_TASK(_) fc::Task<_>
#define _CTX_back(_) co_return _;
#define _CTX_return co_return;
#define _ctx -> fc::Task<>
//co_return
#define _CTX_idx , int& idx
#define _CTX_idex , idex(idx)
#endif
#if _OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
namespace fc {
  static bool RES_OPENSSL_INIT = false;
  struct ssl_context {
    SSL_CTX* ctx = nullptr;
    ssl_context(const std::string& key, const std::string& cert, const std::string& cipher) {
      if (!RES_OPENSSL_INIT) { SSL_load_error_strings(); OpenSSL_add_ssl_algorithms(); RES_OPENSSL_INIT = true; }
      const SSL_METHOD* method = SSLv23_server_method(); ctx = SSL_CTX_new(method);
      if (!ctx) { perror("Fail!"); ERR_print_errors_fp(stderr); exit(EXIT_FAILURE); } SSL_CTX_set_ecdh_auto(ctx, 1); /* Set the cipheruite if provided */
      if (cipher.size() && SSL_CTX_set_cipher_list(ctx, cipher.c_str()) <= 0) { ERR_print_errors_fp(stderr); exit(EXIT_FAILURE); } /* Set the key and cert */
      if (SSL_CTX_use_certificate_file(ctx, cert.c_str(), SSL_FILETYPE_PEM) <= 0) { ERR_print_errors_fp(stderr); exit(EXIT_FAILURE); }
      if (SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0) { ERR_print_errors_fp(stderr); exit(EXIT_FAILURE); }
    }
    ~ssl_context() { if (ctx) SSL_CTX_free(ctx); }
  };
}
#endif
namespace fc {
  static struct linger RESling { 1, 0 };
  enum sd_type { _READ, _WRITE, _BOTH };
  static int RESon = 1; static int RESkeep_AI = 1;//keepalive
#if defined _WIN32
  typedef HANDLE epoll_handle_t;
  typedef UINT_PTR socket_type;//SD_RECEIVE，SD_SEND，SD_BOTH
  static unsigned int RESrcv = 5000;
  static unsigned int RESsed = 10000;
  static tcp_keepalive RESin_kavars{ 0, 0, 0 };
  static tcp_keepalive RESout_kavars{ 0, 0, 0 };
  static unsigned long RESl_k = sizeof(tcp_keepalive);
  static DWORD RESuBR;
#else
  typedef int epoll_handle_t;
  typedef int socket_type;
  static struct timeval RESrcv { 5, 0 };//max{5,0},read
  static struct timeval RESsed { 10, 0 };//write
#endif
  inline int close_socket(socket_type sock) {
#if defined _WIN32
    return closesocket(sock);
#else
    return close(sock);
#endif
  }
  static void epoll_del_cpp20(epoll_handle_t ef, socket_type fd, int& idex) {
#if __linux__ || _WIN32
    epoll_event e; memset(&e, 0, sizeof(e)); e.events = 0; ::epoll_ctl(ef, EPOLL_CTL_DEL, fd, &e); --idex;
#elif __APPLE__
    struct kevent ev_set; EV_SET(&ev_set, fd, 0, EPOLL_CTL_DEL, 0, 0, NULL); kevent(ef, &ev_set, 1, NULL, 0, NULL); --idex;
#endif
  }
  struct ROG {
#if __linux__ || _WIN32
    socket_type $;
#endif
    int64_t hrt;
#if __cplusplus < _cpp20_date
    ctx::co _;
#else
    fc::Task<void> _;
#endif
    u16 on = 2, idx;
  };
  struct Reactor;
  // Epoll based Reactor:
  // Orchestrates a set of fiber (ctx::co).
#if __cplusplus < _cpp20_date
  struct fiber_exception {
    std::string what; ctx::co c; fiber_exception(fiber_exception&& e) = default;
    fiber_exception(ctx::co&& c_, std::string const& what): what{ what }, c{ std::move(c_) } {}
  };
#endif
  class Conn {
    Conn& operator=(const Conn&) = delete; Conn(const Conn&) = delete;
  public:
    sockaddr in_addr;
    fc::timer& timer;
    ROG* rpg;
#if __cplusplus >= _cpp20_date
    int& idex;
#endif
    epoll_handle_t epoll_fd;
    socket_type socket_fd;
#if _OPENSSL
    SSL* ssl = nullptr;
    inline _CTX_TASK(bool) ssl_handshake(std::unique_ptr<ssl_context>& ssl_ctx) {
      ssl = SSL_new(ssl_ctx->ctx); SSL_set_fd(ssl, static_cast<int>(socket_fd));
      int ret, e;
      do {
        ret = SSL_accept(ssl); if (ret == 1) { _CTX_back(true) } e = SSL_get_error(ssl, ret);
        if (e == SSL_ERROR_WANT_WRITE || e == SSL_ERROR_WANT_READ)
#if __cplusplus >= _cpp20_date
          co_await std::suspend_always{};
#else
          rpg->_.operator()();
#endif
        else {
#if _DEBUG
          ERR_print_errors_fp(stderr);
#endif
          _CTX_back(false)
        }
      } while (ret);
      _CTX_back(false)
    }
#endif
    int k_a;
    bool is_idle = true;
    _FORCE_INLINE Conn(socket_type fd, sockaddr a, int k, fc::timer& t, ROG* r, epoll_handle_t e _CTX_idx):
      timer(t), k_a(k), socket_fd(fd), in_addr(a), rpg(r), epoll_fd(e) _CTX_idex {
      r->hrt = RES_TIME_T;
    }
    _FORCE_INLINE ~Conn() {
#if __cplusplus >= _cpp20_date
      if (rpg->on) ++rpg->idx, rpg->on = 0, epoll_del_cpp20(epoll_fd, socket_fd, idex);
#else
      rpg->on = 0;
#endif
#if _OPENSSL
      if (ssl) { SSL_shutdown(ssl); SSL_free(ssl); ssl = nullptr; }
#endif
      close_socket(socket_fd); rpg = nullptr;
    }
    //if (0 != close_socket(socket_fd)) std::cerr << "Error when closing file descriptor " << socket_fd << ": " << strerror(errno) << std::endl;
    _FORCE_INLINE int read_impl(char* buf, int size) {
#if _OPENSSL
      if (ssl) return SSL_read(ssl, buf, size);
#endif
      return ::recv(socket_fd, buf, size, 0);
    }
    _FORCE_INLINE int write_impl(const char* buf, int size) {
#if _OPENSSL
      if (ssl) return SSL_write(ssl, buf, size);
#endif
      return ::send(socket_fd, buf, size, 0);
    }
#if __cplusplus < _cpp20_date
    int read(char* buf, int size);
    bool write(const char* buf, int size);
    bool writen(const char* buf, int size);
#else//The timer must be called externally to destroy the coroutine, which is completely asynchronous and unavailable.
    fc::Task<int> read(char* buf, int max_size) {
      int count = read_impl(buf, max_size); int64_t t = 0;
      while (count < 0) {
#ifndef _WIN32
        if (errno != EAGAIN) { co_return 0; }
#else
        if (errno != EINPROGRESS && errno != EINVAL && errno != ENOENT) { co_return 0; }
#endif // !_WIN32
        co_await std::suspend_always{}; t = time(NULL) - rpg->hrt;
        if (is_idle && t) {
          if (t > k_a) {
#ifdef _WIN32
            ::setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, (const char*)&RESling, sizeof(linger));
#endif
            co_return 0;
          }
          u16 n = ++rpg->idx; ROG* fib = rpg; timer.add_s(k_a + 1, [n, fib] { if (fib->idx == n && fib->_) { fib->_.operator()(); } });
        }
        count = read_impl(buf, max_size);
      }
      co_return count;
    }
    fc::Task<bool> write(const char* buf, int size) {
      const char* end = buf + size; is_idle = false;
      int count = write_impl(buf, size); if (count > 0) buf += count;
      while (buf != end) {
#ifndef _WIN32
        if (count == 0 || count < 0 && errno != EAGAIN) { co_return false; }
#else
        if (count == 0 || count < 0 && (errno != EINVAL && errno != EINPROGRESS)) { co_return false; }
#endif // !_WIN32
        co_await std::suspend_always{};
        if ((count = write_impl(buf, int(end - buf))) > 0) buf += count;
      } time(&rpg->hrt);
      co_return is_idle = true;
    }
    fc::Task<bool> writen(const char* buf, int size) {
      const char* end = buf + size; is_idle = false;
      int count = write_impl(buf, size); if (count > 0) buf += count;
      while (buf != end) {
#ifndef _WIN32
        if (count == 0 || count < 0 && (errno == EPIPE || errno != EAGAIN)) { co_return false; }
#else
        if (count == 0 || count < 0 && (errno == EPIPE || (errno != EINVAL && errno != EINPROGRESS))) { co_return false; }
#endif // !_WIN32
        co_await std::suspend_always{};
        if ((count = write_impl(buf, int(end - buf))) > 0) buf += count;
      }
      co_return is_idle = true;
    };
#endif
    int shut(socket_type fd, sd_type type);
    int shut(sd_type type);
    void epoll_mod(socket_type flags);
  };
}
namespace std {
  template <>struct hash<fc::ROG> {
    size_t operator()(const fc::ROG& o) const {
#if __linux__ || _WIN32
      return hash<fc::socket_type>()(o.$);
#else
      return hash<u16>()(o.idx);
#endif
    }
  };
}
#endif // CONN_HH
