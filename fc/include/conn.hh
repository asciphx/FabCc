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

#include <h/config.h>
#include <tp/ctx.hh>
namespace fc {
  enum sd_type { _READ, _WRITE, _BOTH };
  static int RES_ON = 1;
#if defined _WIN32
  typedef UINT_PTR socket_type;//SD_RECEIVE，SD_SEND，SD_BOTH
  static unsigned int RES_RCV = 5000;
  static unsigned int RES_SED = 10000;
  static tcp_keepalive RES_in_kavars{ 0, 0, 0 };
  static tcp_keepalive RES_out_kavars{ 0, 0, 0 };
  static unsigned long RES_l_k = sizeof(tcp_keepalive);
  static DWORD RES_uBR;
#else
  static int RES_KEEP_Ai = 1;//keepalive
  typedef int socket_type;
  static struct timeval RES_RCV { 5, 0 };//max{5,0},read
  static struct timeval RES_SED { 10, 0 };//write
#endif
  inline int close_socket(socket_type sock) {
#if defined _WIN32
    return closesocket(sock);
#else
    return close(sock);
#endif
  }
  struct Reactor;
  // Epoll based Reactor:
  // Orchestrates a set of fiber (ctx::co).
  struct fiber_exception {
    std::string what; ctx::co c;
    fiber_exception(fiber_exception&& e) = default;
    fiber_exception(ctx::co&& c_, std::string const& what): what{ what }, c{ std::move(c_) } {}
  };
  class Conn {
    Conn& operator=(const Conn&) = delete;
    Conn(const Conn&) = delete;
  public:
    typedef fiber_exception exception_type;
    Reactor* reactor;
    ctx::co sink;
    socket_type socket_fd;
    sockaddr in_addr;
    inline Conn(Reactor* reactor, ctx::co&& sink, socket_type socket_fd, sockaddr in_addr)
      : reactor(reactor), sink(std::move(sink)), socket_fd(socket_fd), in_addr(in_addr) {}
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
    inline ~Conn() {
      //if (ssl) { SSL_shutdown(ssl); SSL_free(ssl); }
      close_socket(socket_fd);
      //if (0 != close_socket(socket_fd)) std::cerr << "Error when closing file descriptor " << socket_fd << ": " << strerror(errno) << std::endl;
    }// Will finally close the fd.
    void epoll_mod(socket_type fd, int flags);
    inline int read_impl(char* buf, int size) {
      //if (ssl) return SSL_read(ssl, buf, size); else
      return ::recv(socket_fd, buf, size, 0);
    }
    inline int write_impl(const char* buf, int size) {
      //if (ssl) return SSL_write(ssl, buf, size); else
      return ::send(socket_fd, buf, size, 0);
    }
    //
    void async(std::function<void()>&& func);
    int read(char* buf, int max_size);
    bool write(const char* buf, int size);
    int shut(socket_type fd, sd_type type);
    int shut(sd_type type);
  };
}
#endif // CONN_HH
