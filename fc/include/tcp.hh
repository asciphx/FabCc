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
  static const int MAXEVENTS = 64;//64
  static volatile int quit_signal_catched = 0;
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
	void event_loop(socket_type& listen_fd, std::function<void(Conn&)> handler) {
#if __linux__
	  this->epoll_fd = epoll_create1(0);
	  epoll_ctl(epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
	  epoll_event events[MAXEVENTS];
#elif  _WIN32
	  this->epoll_fd = epoll_create1(0);
	  epoll_ctl(epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN);
	  epoll_event events[MAXEVENTS];
#elif __APPLE__
	  this->epoll_fd = kqueue();
	  epoll_ctl(this->epoll_fd, listen_fd, EV_ADD, EVFILT_READ);
	  epoll_ctl(this->epoll_fd, SIGINT, EV_ADD, EVFILT_SIGNAL);
	  epoll_ctl(this->epoll_fd, SIGKILL, EV_ADD, EVFILT_SIGNAL);
	  epoll_ctl(this->epoll_fd, SIGTERM, EV_ADD, EVFILT_SIGNAL);
	  struct kevent events[MAXEVENTS];
	  struct timespec timeout;
	  memset(&timeout, 0, sizeof(timeout));
	  timeout.tv_nsec = 10000;
#endif
	  // Main loop.
	  while (!quit_signal_catched) {
#if __linux__ || _WIN32
		int n_events = epoll_wait(epoll_fd, events, MAXEVENTS, 1);
#elif __APPLE__
		// kevent is already listening to quit signals.
		int n_events = kevent(epoll_fd, NULL, 0, events, MAXEVENTS, &timeout);
#endif
		if (n_events == 0) {
		  for (int i = 0; i < fibers.size() && fibers[i]; ++i) fibers[i] = fibers[i].resume();
		}
		for (int i = 0; i < n_events; ++i) {
#if __APPLE__
		  int event_flags = events[i].flags;
		  int event_fd = events[i].ident;
		  if (events[i].filter == EVFILT_SIGNAL) {
			if (event_fd == SIGINT)
			  std::cout << "SIGINT" << std::endl;
			if (event_fd == SIGTERM)
			  std::cout << "SIGTERM" << std::endl;
			if (event_fd == SIGKILL)
			  std::cout << "SIGKILL" << std::endl;
			quit_signal_catched = true;
			break;
		  }
#else
		  socket_type event_flags = events[i].events, event_fd = events[i].data.fd;
#endif
		  // Handle errors on sockets.
#if __linux__ || _WIN32
		  if (event_flags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
#elif __APPLE__
		  if (event_flags & EV_ERROR) {
#endif
			if (event_fd == listen_fd) {
			  std::cout << "FATAL ERROR: Error on server socket " << event_fd << std::endl;
			  quit_signal_catched = true;
			} else {
			  co& fiber = fd_to_fiber(event_fd);
			  if (fiber)
				fiber = fiber.resume_with(std::move([](auto&& sink) {
				throw fiber_exception(std::move(sink), "EPOLLRDHUP");
				return std::move(sink);
				  }));
			}
		  }
		  // Handle new connections.
		  else if (listen_fd == event_fd) {
			// ACCEPT INCOMMING CONNECTION
#ifndef  _WIN32
			while (true) {
#endif
			  socket_type socket_fd = accept(listen_fd, in_addr, &in_len);
#ifdef _WIN32
			  if (socket_fd == INVALID_SOCKET) { break; }
#else
			  if (socket_fd == -1) { break; }
#endif
			  // Subscribe epoll to the socket file descriptor. 将epoll订阅到套接字文件描述符。
#if _WIN32
			  if (ioctlsocket(socket_fd, FIONBIO, &iMode) != NO_ERROR) continue;
#else
			  if (-1 == ::fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK)) continue;
#endif
			  socket_type fiber_idx = 0;// Find a free fiber for this new connection.找到一个空闲的来处理新连接
			  while (fiber_idx < fibers.size() && fibers[fiber_idx]) ++fiber_idx;
			  if (fiber_idx >= fibers.size()) fibers.resize((fibers.size() + 1) * 2);
			  assert(fiber_idx < fibers.size());
#if __linux__
			  this->epoll_add(socket_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, fiber_idx);
#elif _WIN32
			  this->epoll_add(socket_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP, fiber_idx);
#elif __APPLE__
			  this->epoll_add(socket_fd, EVFILT_READ | EVFILT_WRITE, fiber_idx);
#endif
			  // =============================================
			  // Spawn a new co to handle the connection.继续处理，延续之前未处理的
			  fibers[fiber_idx] = ctx::callcc([this, socket_fd, fiber_idx, &handler](co&& sink) {
				Conn c(this, std::move(sink), fiber_idx, socket_fd, *in_addr); c.set_keep_alive(socket_fd, 4, 3, 2);
				scoped_fd sfd{ socket_fd }; // Will finally close the fd.
				try {
				  //if (ssl_ctx && !c.ssl_handshake(this->ssl_ctx)) {
					 //std::cerr << "Error during SSL handshake" << std::endl; return std::move(c.sink);
				  //}
				  handler(c);
				  epoll_del(socket_fd);
				} catch (fiber_exception& ex) {
				  epoll_del(socket_fd); return std::move(ex.c);
				} catch (const std::runtime_error& e) {
				  epoll_del(socket_fd); std::cerr << "Exception in fiber: " << e.what() << std::endl; return std::move(c.sink);
				}
				return std::move(c.sink);
				});
#ifndef _WIN32
			}
#endif
		  } else // Data available on existing sockets. Wake up the fiber associated with event_fd.
		  {
#ifdef _WIN32
			if (event_fd < fd_to_fiber_idx.size()) {
#else
			if (event_fd >= 0 && event_fd < fd_to_fiber_idx.size()) {
#endif
			  co& fiber = fd_to_fiber(event_fd); if (fiber) fiber = fiber.resume();
			} else std::cerr << "Epoll returned a file descriptor that we did not register: " << event_fd << std::endl;
			}
		  // Wakeup fibers if needed. 唤醒功能
		  while (defered_resume.size()) {
			socket_type fiber_id = defered_resume.front(); defered_resume.pop_front();
			assert(fiber_id < fibers.size());
			co& fiber = fibers[fiber_id]; if (fiber) fiber = fiber.resume();
		  }
		  }
		// Call && Flush the defered functions.
		if (defered_functions.size()) { for (auto& f : defered_functions) f(); defered_functions.clear(); }
		  }
	  std::cout << "@";
#if _WIN32
	  epoll_close(epoll_fd);
#else
	  fc::close_socket(epoll_fd);
#endif
		}
	  };
  static void shutdown_handler(int sig) {
	quit_signal_catched = 1;
  }
  inline void start_server(std::string ip, int port, int socktype, int nthreads, std::function<void(Conn&)> conn_handler,
	std::string ssl_key_path = "", std::string ssl_cert_path = "", std::string ssl_ciphers = "") { // Start the winsock DLL
#ifdef _WIN32
	system("chcp 65001 >nul"); setlocale(LC_CTYPE, ".UTF8");
	WSADATA wsaData; int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0) { std::cerr << "WSAStartup failed with error: " << err << std::endl; return; } // Setup quit signals
	signal(SIGINT, shutdown_handler); signal(SIGTERM, shutdown_handler); signal(SIGABRT, shutdown_handler);
#else
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = shutdown_handler;
	sigaction(SIGINT, &act, 0); sigaction(SIGTERM, &act, 0); sigaction(SIGQUIT, &act, 0);
	// Ignore sigpipe signal. Otherwise sendfile causes crashes if the
	// client closes the connection during the response transfer.
#endif
#if __linux__
	signal(SIGPIPE, SIG_IGN);
#endif
	// Start the server threads.
	const char* listen_ip = !ip.empty() ? ip.c_str() : nullptr;
	socket_type server_fd = fc::create_and_bind(listen_ip, port, socktype);
	std::vector<std::thread> ths;
	for (int i = 0; i < nthreads; ++i) {
	  ths.push_back(std::thread([&] {
		Reactor reactor;
		// if (ssl_cert_path.size()) // Initialize the SSL/TLS context.
		   //reactor.ssl_ctx = std::make_unique<ssl_context>(ssl_key_path, ssl_cert_path, ssl_ciphers);
		reactor.event_loop(server_fd, conn_handler);
		}));
	}
	for (auto& t : ths) t.join();
	fc::close_socket(server_fd); std::cout << std::endl;
  }
	}
#endif
