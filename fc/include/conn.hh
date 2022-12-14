#ifndef CONN_HH
#define CONN_HH
#include <iostream>
#include <errno.h>
#include <fcntl.h>
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <deque>
#include <thread>
#include <vector>

#include <h/config.h>
#include <tp/ctx.hh>
namespace fc {
  enum sd_type { _READ, _WRITE, _BOTH };
#if defined _WIN32
  typedef UINT_PTR socket_type;//SD_RECEIVE，SD_SEND，SD_BOTH
  static unsigned int RES_RCV = 5000;
  static unsigned int RES_SED = 10000;
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
	socket_type fiber_id, socket_fd;
	sockaddr in_addr;
	inline Conn(Reactor* reactor, ctx::co&& sink,
	  socket_type fiber_id, socket_type socket_fd, sockaddr in_addr)
	  : reactor(reactor), sink(std::forward<ctx::co&&>(sink)),
	  fiber_id(fiber_id), socket_fd(socket_fd), in_addr(in_addr) {}
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
	//	  this->sink.resume();
	//	else {
	//	  ERR_print_errors_fp(stderr);
	//	  return false;// throw std::runtime_error("Error during https handshake.");
	//	}
	//  }
	//  return false;
	//}
	inline ~Conn() {
	  //if (ssl) { SSL_shutdown(ssl); SSL_free(ssl); }
	}
	void epoll_add(socket_type fd, int flags);
	void epoll_mod(socket_type fd, int flags);
	void reassign_fd_to_this_fiber(socket_type fd);
	void defer_fiber_resume(socket_type fiber_id);
	void defer(const std::function<void()>& fun);
	inline int read_impl(char* buf, int size) {
	  //if (ssl) return SSL_read(ssl, buf, size); else
	  return ::recv(socket_fd, buf, size, 0);
	}
	inline int write_impl(const char* buf, int size) {
	  //if (ssl) return SSL_write(ssl, buf, size); else
	  return ::send(socket_fd, buf, size, 0);
	}
	//
	int read(char* buf, int max_size);
	bool write(const char* buf, int size);
	int shut(socket_type fd, sd_type type);
	int shut(sd_type type);
	int set_keep_alive(socket_type fd, int idle, int intvl = 1, unsigned char probes = 10);
  };
}
#endif // CONN_HH
