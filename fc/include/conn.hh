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
#if defined _WIN32
  typedef UINT_PTR socket_type;
#else
  typedef int socket_type;
#endif
  inline int close_socket(socket_type sock) {
#if defined _WIN32
	return closesocket(sock);
#else
	return close(sock);
#endif
  }
  static const int MAXEVENTS = 64;
  static volatile int quit_signal_catched = 0;
  static void shutdown_handler(int sig) { quit_signal_catched = 1; }
  // Epoll based Reactor:
  // Orchestrates a set of fiber (ctx::co).
  struct fiber_exception {
	std::string what; ctx::co c;
	fiber_exception(fiber_exception&& e) = default;
	fiber_exception(ctx::co&& c_, std::string const& what) : what{ what }, c{ std::move(c_) } {}
  };
  class Conn {
	Conn& operator=(const Conn&) = delete;
	Conn(const Conn&) = delete;
  public:
	typedef fiber_exception exception_type;
	void* reactor;
	ctx::co sink;
	socket_type fiber_id, socket_fd;
	sockaddr in_addr;
	inline Conn(void* reactor, ctx::co&& sink,
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
	inline int read(char* buf, int max_size) {
	  int count = read_impl(buf, max_size);
	  while (count <= 0) {
		if ((count < 0 && errno != EAGAIN) || count == 0) return int(0);
		sink = sink.yield();	count = read_impl(buf, max_size);
	  }
	  return count;
	};
	inline bool write(const char* buf, int size) {
	  if (!buf || !size) {
		sink = sink.yield(); return true;
	  }
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
  };
}
#endif // CONN_HH
