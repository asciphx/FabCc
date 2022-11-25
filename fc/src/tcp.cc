#include "tcp.hh"
#include "app.hh"
#include <lexical_cast.hh>
#include <h/common.h>
#include <http_error.hh>
#pragma warning(disable: 4996)
namespace fc {
  static const int MAXEVENTS = 64;
  static void shutdown_handler(int sig) { quit_signal_catched = 1; }
  socket_type create_and_bind(const char* ip, int port, int socktype) {
	int s; socket_type sfd;
	if (ip == nullptr) {
	  // No IP address was specified, find an appropriate one
	  struct addrinfo hints, * result, * rp;
	  char port_str[20];
	  snprintf(port_str, sizeof(port_str), "%d", port);
	  memset(&hints, 0, sizeof(struct addrinfo));
	  // On windows, setting up the dual-stack mode (ipv4/ipv6 on the same socket).
	  // https://docs.microsoft.com/en-us/windows/win32/winsock/dual-stack-sockets
	  hints.ai_family = AF_INET6;
	  hints.ai_socktype = socktype; /* We want a TCP socket */
	  hints.ai_flags = AI_PASSIVE;  /* All interfaces */
	  s = getaddrinfo(NULL, port_str, &hints, &result);
	  if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s)); return -1;
	  }
	  for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1) continue;
		// Turn of IPV6_V6ONLY to accept ipv4.
		// https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections
		int ipv6only = 0;
		if (setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only)) != 0) {
		  std::cerr << "FATAL ERROR: setsockopt error when setting IPV6_V6ONLY to 0: " << strerror(errno) << std::endl;
		}
		s = ::bind(sfd, rp->ai_addr, int(rp->ai_addrlen));
		if (s == 0) {
		  /* We managed to bind successfully! */
		  break;
		} else {
		  close_socket(sfd);
		}
	  }
	  if (rp == NULL) {
		fprintf(stderr, "Could not bind: %s\n", strerror(errno)); return -1;
	  }
	  freeaddrinfo(result);
	} else {
	  sfd = socket(AF_INET, socktype, 0);
	  // Use the user specified IP address
	  struct sockaddr_in addr;
	  addr.sin_family = AF_INET;
	  addr.sin_addr.s_addr = inet_addr(ip);
	  addr.sin_port = port;
	  s = ::bind(sfd, (struct sockaddr*)&addr, sizeof(addr));
	  if (s != 0) {
		fprintf(stderr, "Could not bind: %s\n", strerror(errno)); close_socket(sfd); return -1;
	  }
	}
#if _WIN32
	u_long set_on = 1;
	auto ret = ioctlsocket(sfd, FIONBIO, &set_on);
	if (ret) {
	  std::cerr << "FATAL ERROR: Cannot set socket to non blocking mode with ioctlsocket" << std::endl;
	}
#else
	int flags = fcntl(sfd, F_GETFL, 0);
	fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
#endif
	::listen(sfd, SOMAXCONN);
	return sfd;
  }
  co& Reactor::fd_to_fiber(socket_type fd) {
	assert(fd >= 0 && fd < fd_to_fiber_idx.size());
	socket_type fiber_idx = fd_to_fiber_idx[fd];
	assert(fiber_idx >= 0 && fiber_idx < fibers.size());
	return fibers[fiber_idx];
  }
  void Reactor::reassign_fd_to_fiber(socket_type fd, socket_type fiber_idx) { fd_to_fiber_idx[fd] = fiber_idx; }
  void Reactor::epoll_ctl(epoll_handle_t epoll_fd, socket_type fd, int action, socket_type flags) {
#if __linux__ || _WIN32
	epoll_event event;
	memset(&event, 0, sizeof(event));
	event.data.fd = fd; event.events = flags;
	if (-1 == ::epoll_ctl(epoll_fd, action, fd, &event) && errno != EEXIST)
	  std::cout << "epoll_ctl error: " << strerror(errno) << std::endl;
#elif __APPLE__
	struct kevent ev_set;
	EV_SET(&ev_set, fd, flags, action, 0, 0, NULL);
	kevent(epoll_fd, &ev_set, 1, NULL, 0, NULL);
#endif
  };
#if _WIN32
  void Reactor::epoll_add(socket_type new_fd, int flags, socket_type fiber_idx) {
	epoll_ctl(epoll_fd, new_fd, EPOLL_CTL_ADD, flags);
#elif __linux__
  void Reactor::epoll_add(socket_type new_fd, int flags, socket_type fiber_idx) {
	epoll_ctl(epoll_fd, new_fd, EPOLL_CTL_ADD, flags);
#elif __APPLE__
  void Reactor::epoll_add(socket_type new_fd, int flags, socket_type fiber_idx) {
	epoll_ctl(epoll_fd, new_fd, EV_ADD, flags);
#endif
	// Associate new_fd to the fiber.找到一个新的传输光纤id
	if (fd_to_fiber_idx.size() < new_fd + 1) fd_to_fiber_idx.resize((new_fd + 1) * 2, -1);
	fd_to_fiber_idx[new_fd] = fiber_idx;
  }
  void Reactor::epoll_del(socket_type fd) {
	epoll_ctl(epoll_fd, fd, EPOLL_CTL_DEL, 0);
  }
  void Reactor::epoll_mod(socket_type fd, int flags) {
#if __linux__ || _WIN32
	epoll_ctl(epoll_fd, fd, EPOLL_CTL_MOD, flags);
#elif __APPLE__
	epoll_ctl(epoll_fd, fd, EV_ADD, flags);
#endif
  }
  void Reactor::event_loop(socket_type & listen_fd, std::function<void(Conn&)> handler) {
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
			  Conn c(this, std::move(sink), fiber_idx, socket_fd, *in_addr);
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
	std::cout << "@" << std::endl;
#if _WIN32
	epoll_close(epoll_fd);
#else
	close(epoll_fd);
#endif
  }
  void start_server(std::string ip, int port, int socktype, int nthreads, std::function<void(Conn&)> conn_handler,
  std::string ssl_key_path, std::string ssl_cert_path, std::string ssl_ciphers) { // Start the winsock DLL
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
	// Start the server threads.
	const char* listen_ip = !ip.empty() ? ip.c_str() : nullptr;
	socket_type server_fd = fc::create_and_bind(listen_ip, port, socktype);
	std::vector<std::thread> ths;
	for (int i = 0; i < nthreads; ++i) {
	  ths.push_back(std::thread([&server_fd, &conn_handler] {
		Reactor reactor;
		// if (ssl_cert_path.size()) // Initialize the SSL/TLS context.
		   //reactor.ssl_ctx = std::make_unique<ssl_context>(ssl_key_path, ssl_cert_path, ssl_ciphers);
		reactor.event_loop(server_fd, conn_handler);
		}));
	}
	for (auto& t : ths) t.join();
	fc::close_socket(server_fd);
  }
}
