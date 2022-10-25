#include "tcp.hh"
#include <lexical_cast.hh>
#include <timer.hh>
#include <h/common.h>
#include <http_error.hh>
#pragma warning(disable: 4996)
namespace fc {
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
	u_long set_on = 1;// printf("!!!");
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
	  if (defered_functions.size()) { for (std::function<void()>& f : defered_functions) f(); defered_functions.clear(); }
	}
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
	fc::close_socket(server_fd);
  }
}

//  Tcp::Tcp(App* app, uv_loop_t* loop):opened(false), loop_(loop), addr_len(16), app_(app)
//	, roundrobin_index_(std::thread::hardware_concurrency())
//	{
//#ifdef _WIN32
//	::system("chcp 65001 >nul"); setlocale(LC_CTYPE, ".UTF8");
//#else
//	std::locale::global(std::locale("en_US.UTF8"));
//#endif
//	time(&RES_TIME_T); RES_NOW = localtime(&RES_TIME_T); RES_NOW->tm_isdst = 0; RES_last = uv_now(loop);
//	RES_DATE_STR.resize(0x30); RES_DATE_STR.resize(strftime(&RES_DATE_STR[0], 0x2f, RES_GMT, RES_NOW));
//	if (app_ != nullptr) { app_->content_types = &content_types; } _.data = this;
//  }
//  Tcp& Tcp::timeout(unsigned short m) {
//	keep_milliseconds = m < 301 ? m * 1000 : m < 0x640 ? m *= 6 : m; return *this;
//  }
//  Tcp& Tcp::upload_path(std::string p) {
//	if (p.back() != '\\' && p.back() != '/') p += '/'; detail::upload_path_ = p; return *this;
//  }
//  Tcp& Tcp::file_type(const std::vector<std::string_view>& line) {
//	for (std::vector<std::string_view>::const_iterator iter = line.cbegin(); iter != line.cend(); ++iter) {
//	  std::string_view sv; sv = content_any_types[*iter];
//	  if (sv != "") { content_types.emplace(*iter, sv); } else { content_types.emplace(*iter, RES_oct); }
//	} not_set_types = false; return *this;
//  }
//  Tcp& Tcp::setThread(char n) {
//	uv_cpu_info_t* uc; int cpu; uv_cpu_info(&uc, &cpu); uv_free_cpu_info(uc, cpu);
//	if (n > cpu) n = cpu; threads = n > 0 ? n : cpu; return *this;
//  }
//  Tcp& Tcp::maxConnection(int backlog) { max_conn = backlog < 0 ? 1 : backlog; return *this; }
//  void Tcp::exit() { if (!opened)return; opened = false; uv_close((uv_handle_t*)&_, NULL); uv_stop(loop_); uv_loop_close(loop_); }// uv_loop_close(loop_);
//  Tcp::~Tcp() { uv_mutex_destroy(&RES_MUTEX); exit(); }
//  Tcp& Tcp::router(App& app) { app_ = &app; app_->content_types = &content_types; return *this; }
//  Tcp& Tcp::setTcpNoDelay(bool enable) { uv_tcp_nodelay(&_, enable ? 1 : 0); return *this; }
//  bool Tcp::bind(const char* ip_addr, int port, bool is_ipv4) {
//	std::cout << "C++<web>[" << static_cast<int>(threads) << "] => http://127.0.0.1:" << port << std::endl; int $; if (is_ipv4) {
//	  struct sockaddr_in addr; $ = uv_ip4_addr(ip_addr, port, &addr); if ($)return false;
//	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = false;
//	} else {
//	  struct sockaddr_in6 addr; $ = uv_ip6_addr(ip_addr, port, &addr); if ($)return false;
//	  $ = uv_tcp_bind(&_, (const struct sockaddr*)&addr, 0); is_ipv6 = true; addr_len = sizeof(sockaddr_storage);
//	} return $ ? true : false;
//  }
//  bool Tcp::Start(const char* ip_addr, int port, bool is_ipv4) {
//	if (opened)return false; opened = true; if (!loop_)return false; if (uv_tcp_init(loop_, &_)) return false;
//	if (!port)port = port_; core_ = threads - 1; async_ = (uv_async_t*)malloc(sizeof(uv_async_t));
//	if (bind(ip_addr, port, is_ipv4))return false;
//	if (!is_directory(detail::directory_)) create_directory(detail::directory_); uv_mutex_init_recursive(&RES_MUTEX);
//	std::string s(detail::directory_ + detail::upload_path_); if (!is_directory(s)) create_directory(s);
//	if (not_set_types) content_types = content_any_types, not_set_types = false;
//	//if (not_set_types)file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt" }), not_set_types = false;
//#ifdef SIGPIPE
//	signal(SIGPIPE, SIG_IGN);
//#endif
//	if (uv_listen((uv_stream_t*)&_, max_conn, on_conn))return false; uv_async_init(loop_, async_, on_async_cb); async_->data = this;
//	uv_run(loop_, UV_RUN_DEFAULT); uv_loop_close(loop_); return true;
//  }
//  void Tcp::on_async_cb(uv_async_t* srv) {
//	Tcp* t = static_cast<Tcp*>(srv->data);
//  }
//  void Tcp::read_cb(uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) {
//	Conn* c = (Conn*)h->data;
//	if (nread > 0) {
//	  int failed = llhttp__internal_execute(&c->parser_, buf->base, buf->base + nread);
//	  if (failed) { uv_close((uv_handle_t*)h, on_close); return; } Req& req = c->req_;
//	  req = std::move(c->parser_.to_request());// printf("(%s,%s) ",req.url.c_str(),m2c(req.method));
//	  if (!req.url(0)) return;
//#if defined __linux__ || defined __APPLE__
//	  if (c->parser_.http_minor != 0 && STR_KEY_EQ(get_header(req.headers, RES_Con), "close")) {
//		uv_close((uv_handle_t*)h, on_close); return;
//	  }
//#endif
//	  Res& res = c->res_; LOG_GER(m2c(req.method) << " |" << res.code << "| " << req.url);// c->_.data = &res;
//	  res.timer_.setTimeout([h, c] {
//		c->shut(_BOTH); uv_close((uv_handle_t*)h, on_close);
//	  }, c->keep_milliseconds);// res.add_header(RES_Con, "Keep-Alive");
//	  if (uv_now(c->loop_) - RES_last > 1000) {//uv_mutex_lock(&RES_MUTEX); uv_mutex_unlock(&RES_MUTEX);
//		time(&RES_TIME_T); RES_last = uv_now(c->loop_);
//#if defined(_MSC_VER) || defined(__MINGW32__)
//		localtime_s(RES_NOW, &RES_TIME_T);
//#else
//		localtime_r(&RES_TIME_T, RES_NOW);
//#endif
//		RES_DATE_STR.resize(strftime(&RES_DATE_STR[0], 0x2f, RES_GMT, RES_NOW));
//	  } //printf("<%s,%lld> ", req.params.c_str(), req.uuid);
//	  Buf& s = c->buf_;
//	  try {
//		((App*)c->app_)->_call(req.method, req.url, req, res);
//		//for (std::pair<const Buf, Buf>& kv : req.headers) std::cout << kv.first << RES_seperator << kv.second << RES_crlf;
//		if (res.is_file > 0) {
//		  if (res.is_file == 1) {
//			c->set_status(res, res.code); s << RES_http_status << c->status_;
//			res.is_file = 0; req.uuid = hackUrl(req.url.c_str());
//			s << RES_CE << RES_seperator << RES_gzip << RES_crlf; s.append("ID: ", 4) << c->id << RES_crlf;
//			if (RES_CACHE_TIME[req.uuid] > nowStamp()) { res.body << RES_CACHE_MENU[req.uuid]; goto _; }
//			RES_CACHE_TIME[req.uuid] = nowStamp(CACHE_HTML_TIME_SECOND);
//			s << RES_CT << RES_seperator << RES_Txt << RES_crlf; s << RES_date_tag << RES_DATE_STR << RES_crlf;
//			std::ifstream inf(res.path_, std::ios::in | std::ios::binary);
//			int l = 0; $:l = inf.read(c->readbuf, sizeof(c->readbuf)).gcount();
//			if (l) { res.body << res.compress_str(c->readbuf, l); goto $; } inf.close();
//			s << RES_content_length_tag << res.body.size() << RES_crlf;
//			s << RES_crlf << res.body; c->write(s.data_, s.size()); s.reset();
//			RES_CACHE_MENU[req.uuid] = res.body; res.zlib_cp_str.reset(); res.body.reset(); return;
//		  }
//		  Buf range = get_header(req.headers, RES_Range);
//		  if (range != "") res.code = 206;//wait fix 206, not support...
//		  c->set_status(res, res.code); s << RES_http_status << c->status_; s.append("ID: ", 4) << c->id << RES_crlf;
//		  for (std::pair<const Buf, Buf>& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
//		  s << RES_AR << RES_seperator << RES_bytes << RES_crlf;
//		  if (res.code == 206) {
//			long i{ 0 }; range = range.substr(6).pop_back(); i = std::lexical_cast<long>(range); LOG_GER(' ' << range << ',');
//			s << RES_content_length_tag << std::to_string(res.file_size - i) << RES_crlf;
//			s << RES_CR << RES_seperator << RES_bytes << ' ' << i << '-' << (res.file_size - 1) << '/' << res.file_size << RES_crlf;
//			s << RES_crlf;
//			res.is_file = 0; res.headers.clear();
//			if (!c->write(s.data_, s.size())) { s.reset(); return; }; s.reset();
//			if (res.__->ptr_ != nullptr) {
//			  //c->write(res.__->ptr_, static_cast<int>(res.file_size), 1);
//			  int r = res.__->read_chunk(0, res.file_size, [c](const char* s, size_t l, std::function<void()> d) {
//				if (l > 0) { c->write(s, static_cast<int>(l)); if (d != nullptr) d(); } });
//			  if (r == EOF) c->write(nullptr, 0);
//			}
//			return;
//		  }
//		  s << RES_content_length_tag << std::to_string(res.file_size) << RES_crlf;
//		  s << RES_Ca << RES_seperator << FILE_TIME << RES_crlf << RES_Xc << RES_seperator << RES_No << RES_crlf;
//		  s << RES_crlf;
//		  res.is_file = 0; res.headers.clear();
//		  if (!c->write(s.data_, s.size())) { s.reset(); return; }; s.reset();
//		  if (res.__->ptr_ != nullptr) {
//			//c->write(res.__->ptr_, static_cast<int>(res.file_size));
//			int r = res.__->read_chunk(0, res.file_size, [c](const char* s, size_t l, std::function<void()> d) {
//			  if (l > 0) { c->write(s, static_cast<int>(l)); if (d != nullptr) d(); } });
//			if (r == EOF) c->write(nullptr, 0);
//		  }
//		  return;
//		}
//		c->set_status(res, res.code); s << RES_http_status << c->status_;
//	  } catch (const http_error& e) {
//		s.reset(); c->set_status(res, e.i()); res.body = e.what(); s << RES_http_status << c->status_; goto _;
//	  } catch (const std::exception& e) {
//		s.reset(); c->set_status(res, 500); res.body = e.what(); s << RES_http_status << c->status_; goto _;
//	  }
//	  //if (STR_KEY_EQ(get_header(req.headers, RES_Ex), "100-continue") &&
//	  // c->parser_.http_major == 1 && c->parser_.http_minor == 1)s << expect_100_continue;
//	  for (std::pair<const Buf, Buf>& kv : res.headers) s << kv.first << RES_seperator << kv.second << RES_crlf;
//#ifdef AccessControlAllowCredentials
//	  s << RES_AcC << AccessControlAllowCredentials << RES_crlf;
//#endif
//#ifdef AccessControlAllowHeaders
//	  s << RES_AcH << AccessControlAllowHeaders << RES_crlf;
//#endif
//#ifdef AccessControlAllowMethods
//	  s << RES_AcM << AccessControlAllowMethods << RES_crlf;
//#endif
//#ifdef AccessControlAllowOrigin
//	  s << RES_AcO << AccessControlAllowOrigin << RES_crlf;
//#endif
//      s.append("ID: ", 4) << c->id << RES_crlf;
//	_:
//#if SHOW_SERVER_NAME
//	  s << RES_server_tag << SERVER_NAME << RES_crlf;
//#endif
//	  if (!res.headers.count(RES_CT)) { s << RES_CT << RES_seperator << RES_Txt << RES_crlf; }
//	  s << RES_content_length_tag << res.body.size() << RES_crlf;
//	  s << RES_date_tag << RES_DATE_STR << RES_crlf;
//	  s << RES_crlf << res.body; res.headers.clear(); res.code = 200; res.body.reset();
//	  DEBUG("客户端：%lld %s \n", c->id, s.c_str());
//	  c->write(s.data_, s.size()); s.reset();
//	} else if (nread < 0) {
//	  if (nread == UV_EOF || nread == UV_ECONNRESET) {
//		DEBUG("1->%Id: %s : %s\n", nread, uv_err_name(nread), uv_strerror(nread));
//		uv_close((uv_handle_t*)h, on_close);
//	  } else {//UV_ENOBUFS
//		DEBUG("2->%Id: %s : % s\n", nread, uv_err_name(nread), uv_strerror(nread));
//		uv_close((uv_handle_t*)h, on_close);
//	  }//if (!uv_is_active((uv_handle_t*)h))// uv_read_stop((uv_stream_t*)h);
//	}//printf("%d\n", nread);uv_read_stop(h);
//  }
//  void Tcp::alloc_cb(uv_handle_t* h, size_t suggested, uv_buf_t* b) { Conn* c = (Conn*)h->data; *b = c->rbuf; }
//  void Tcp::on_close(uv_handle_t* h) {
//	Conn* c = (Conn*)h->data; --((Tcp*)c->tcp_)->connection_num; DEBUG("{x%Id}\n", c->id);
//	((Tcp*)c->tcp_)->$.erase(static_cast<unsigned int>(c->id)); delete c;
//  }
//  void Tcp::on_conn(uv_stream_t* srv, int st) {
//	Tcp* t = (Tcp*)srv->data; u16 idex = t->pick_io_tcp(); ++t->roundrobin_index_[idex];
//	Conn* c = new Conn(t->keep_milliseconds, t->loop_, t->roundrobin_index_[idex]);
//	c->app_ = t->app_; c->tcp_ = t; uv_tcp_init(t->loop_, &c->slot_);
//	int $ = uv_accept((uv_stream_t*)&t->_, (uv_stream_t*)&c->slot_);
//	if ($) { uv_close((uv_handle_t*)&c->slot_, NULL); delete c; return; }
//	if (0 == uv_tcp_getpeername((uv_tcp_t*)&c->slot_, (sockaddr*)&t->addr_, &t->addr_len)) {
//	  if (!t->is_ipv6) {
//		sockaddr_in addr = *((sockaddr_in*)&t->addr_); char name[16] = {};
//		uv_inet_ntop(addr.sin_family, &addr.sin_addr, name, t->addr_len); c->req_.ip_addr.append(name, t->addr_len);
//	  } else {
//		sockaddr_in6 addr = *((sockaddr_in6*)&t->addr_); char name[128] = {};
//		uv_inet_ntop(addr.sin6_family, &addr.sin6_addr, name, t->addr_len); c->req_.ip_addr.append(name, t->addr_len);
//	  }
//	}
//	++t->connection_num; c->set_keep_alive(c->id, 3, 2, 3);
//#ifdef _WIN32
//	c->id = c->slot_.socket;
//#else
//	c->id = uv__stream_fd(&c->slot_);
//#endif
//	t->$.insert(static_cast<unsigned int>(c->id));
//	uv_read_start((uv_stream_t*)&c->slot_, alloc_cb, read_cb);
//  }