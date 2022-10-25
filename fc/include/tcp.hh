#ifndef TCP_HH
#define TCP_HH
#define DEFAULT_ENABLE_LOGGING 0
#include <string>
#include <map>
#include <stdio.h>
#include <future>
#include <atomic>
#include <thread>
#include <vector>
#include <fstream>
#include <set>
#include <conn.hh>
#include <buf.hh>
namespace fc {
  /**
   * Open a socket on port \port && call \conn_handler(int client_fd, auto read, auto write)
   * to process each incomming connection. This handle takes 3 argments:
   *               - int client_fd: the file descriptor of the socket.
   *               - int read(buf, max_size_to_read):
   *                       The callback that conn_handler can use to read on the socket.
   *                       If data is available, copy it into \buf, otherwise suspend the handler
   * until there is something to read. Returns the number of char actually read, returns 0 if the
   * connection has been lost.
   *               - bool write(buf, buf_size): return true on success, false on error.
   *                       The callback that conn_handler can use to write on the socket.
   *                       If the socket is ready to write, write \buf, otherwise suspend the handler
   * until it is ready. Returns true on sucess, false if connection is lost.
   * @param port  The server port.
   * @param socktype The socket type, SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
   */
   // Helper to create a TCP/UDP server socket.
  socket_type create_and_bind(const char* ip, int port, int socktype);
  // Simply utility to close fd at the end of a scope.
  struct scoped_fd {
	socket_type fd;
	~scoped_fd() {
	  if (0 != fc::close_socket(fd)) std::cerr << "Error when closing file descriptor " << fd << ": " << strerror(errno) << std::endl;
	}
  };
  struct Reactor {
	sockaddr_storage in_addr_storage;
	socklen_t in_len = sizeof(sockaddr_storage);
	sockaddr* in_addr = (sockaddr*)&in_addr_storage;
#if defined _WIN32
	typedef HANDLE epoll_handle_t;
	u_long iMode = 0;
#else
	typedef int epoll_handle_t;
#endif
	epoll_handle_t epoll_fd;
	std::vector<co> fibers;
	std::vector<socket_type> fd_to_fiber_idx;
	std::vector<std::function<void()>> defered_functions;
	std::deque<socket_type> defered_resume;
	//std::unique_ptr<ssl_context> ssl_ctx = nullptr;
	co& fd_to_fiber(socket_type fd);
	void reassign_fd_to_fiber(socket_type fd, socket_type fiber_idx);
	void epoll_ctl(epoll_handle_t epoll_fd, socket_type fd, int action, socket_type flags);
#if _WIN32
	void epoll_add(socket_type new_fd, int flags, socket_type fiber_idx = INVALID_SOCKET);
#elif __linux__ || __APPLE__
	void epoll_add(socket_type new_fd, int flags, socket_type fiber_idx = -1);
#endif
	void epoll_del(socket_type fd);
	void epoll_mod(socket_type fd, int flags);
	void event_loop(socket_type& listen_fd, std::function<void(Conn&)> handler);
  };
  void start_server(std::string ip, int port, int socktype, int nthreads, std::function<void(Conn&)> conn_handler,
	std::string ssl_key_path = "", std::string ssl_cert_path = "", std::string ssl_ciphers = "");
}
#endif
