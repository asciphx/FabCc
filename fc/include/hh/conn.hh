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
#include <h/wepoll.h>
#include <mstcpip.h>
#endif
#include <chrono>
#include <atomic>
#include <h/config.h>
#include <tp/ctx.hh>
#include <hpp/http_top_header_builder.hpp>
namespace fc {
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
  struct ROG {
#if __linux__ || _WIN32
    socket_type $;
#endif
    co _; u16 idx;
  };
  struct Reactor;
  // Epoll based Reactor:
  // Orchestrates a set of fiber (ctx::co).
  struct fiber_exception {
    std::string what; ctx::co c; fiber_exception(fiber_exception&& e) = default;
    fiber_exception(ctx::co&& c_, std::string const& what): what{ what }, c{ std::move(c_) } {}
  };
  class Conn {
    Conn& operator=(const Conn&) = delete; Conn(const Conn&) = delete;
  public:
    sockaddr in_addr;
    fc::timer& timer;
    ROG* rpg;
    int64_t hrt;
    epoll_handle_t epoll_fd;
    socket_type socket_fd;
    int k_a;
    bool is_f = false, is_idle = true;
    _FORCE_INLINE Conn(socket_type fd, sockaddr a, int k, fc::timer& t, ROG* r, epoll_handle_t e):
      timer(t), k_a(k), socket_fd(fd), in_addr(a), hrt(RES_TIME_T), rpg(r), epoll_fd(e) {}
    _FORCE_INLINE ~Conn() {
      //if (ssl) { SSL_shutdown(ssl); SSL_free(ssl); }
      close_socket(socket_fd); rpg = nullptr;
    }

    //if (0 != close_socket(socket_fd)) std::cerr << "Error when closing file descriptor " << socket_fd << ": " << strerror(errno) << std::endl;
    //SSL* ssl = nullptr;
    //inline bool ssl_handshake(std::unique_ptr<ssl_context>& ssl_ctx) {
    //  if (!ssl_ctx)
    //	return false;
    //  ssl = SSL_new(ssl_ctx->ctx);
    //  SSL_set_fd(ssl, socket_fd);
    //  while (int ret = SSL_accept(ssl)) {
    //	if (ret == 1)
    //	  return true;
    //	int err = SSL_get_error(ssl, ret);
    //	if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ)
    //	  this->sink.yield();
    //	else {
    //	  ERR_print_errors_fp(stderr);
    //	  return false;// throw std::runtime_error("Error during https handshake.");
    //	}
    //  }
    //  return false;
    //}
    _FORCE_INLINE int read_impl(char* buf, int size) {
      //if (ssl) return SSL_read(ssl, buf, size); else
      return ::recv(socket_fd, buf, size, 0);
    }
    _FORCE_INLINE int write_impl(const char* buf, int size) {
      //if (ssl) return SSL_write(ssl, buf, size); else
      return ::send(socket_fd, buf, size, 0);
    }
    //
    int read(char* buf, int max_size);
    bool write(const char* buf, int size);
    bool writen(const char* buf, int size);
    int shut(socket_type fd, sd_type type);
    int shut(sd_type type);
    void epoll_mod(socket_type flags);
  };
}
namespace std {
  template <>struct hash<fc::ROG> { size_t operator()(const fc::ROG& o) const { return hash<fc::socket_type>()(o.$); } };
}
#endif // CONN_HH
