#ifndef TCP_HH
#define TCP_HH
#define DEFAULT_ENABLE_LOGGING 0
#include <string>
#include <map>
#include <stdio.h>
#include <future>
#include <atomic>
#include <thread>
#include <stack>
#include <vector>
#include <fstream>
#include <set>
#include <mutex>
#include "hh/conn.hh"
#include "hpp/sp_tree.hpp"
#include "hpp/box.hpp"
#pragma warning(disable:4244)
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
  static int RESmaxEVENTS = 16; static std::once_flag RESonce_flag;
  static volatile int RESquit_signal_catched = 1;
  static std::stack<std::future<void>> RESfus;
  static void create_init(int n) {};
#if _WIN32
  __declspec(selectany) http_top_header_builder REStop_h;
  static u_long RESiMode = 1;
#else
  http_top_header_builder REStop_h [[gnu::weak]];
#endif
  struct Reactor {
    sockaddr_storage in_addr_storage;
    fc::timer loop_timer;
    epoll_handle_t epoll_fd;
#if __linux__ || _WIN32
    epoll_event* kevents;
#elif __APPLE__
    kevent* kevents;
#endif
    sockaddr* in_addr = (sockaddr*)&in_addr_storage;
    std::chrono::steady_clock::time_point t{ RES_TP };
    socklen_t in_len{ sizeof(sockaddr_storage) };
  #ifdef __MINGW32__
    struct stat64 statbuf_;
  #else
    struct stat statbuf_;
  #endif
    socket_type event_flags, event_fd;
    int n_events, i;
#if _OPENSSL
    std::unique_ptr<ssl_context> ssl_ctx = nullptr;
#endif
    void epoll_ctl(epoll_handle_t epoll_fd, socket_type fd, int action, socket_type flags, void* ptr = NULL);
    _FORCE_INLINE void epoll_del(socket_type fd) {
#if __linux__ || _WIN32
      epoll_event e{ 0 }; ::epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, &e);
#elif __APPLE__
      struct kevent e; EV_SET(&e, fd, 0, EPOLL_CTL_DEL, 0, 0, NULL); kevent(this->epoll_fd, &e, 1, NULL, 0, NULL);
#endif
#if _WIN32
      ::setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&RESling, sizeof(linger));
#endif
    }
    void event_loop(socket_type listen_fd, std::function<_CTX_FUNC> handler, int nthreads, int k_a, int* k_A, int ids, void* ap) {
      std::call_once(RESonce_flag, create_init, k_a); ROG rpg;
#if __linux__
      this->epoll_fd = epoll_create1(EPOLL_CLOEXEC); epoll_ctl(this->epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET, &rpg);
      this->kevents = static_cast<epoll_event*>(calloc(RESmaxEVENTS, sizeof(epoll_event))); rpg.$ = listen_fd;
#elif  _WIN32
      this->epoll_fd = epoll_create(); epoll_ctl(this->epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN, &rpg);
      this->kevents = static_cast<epoll_event*>(calloc(RESmaxEVENTS, sizeof(epoll_event))); rpg.$ = listen_fd;
#elif __APPLE__
      this->epoll_fd = kqueue(); this->kevents = static_cast<kevent*>(calloc(RESmaxEVENTS, sizeof(kevent)));
      epoll_ctl(this->epoll_fd, listen_fd, EV_ADD, EVFILT_READ, &rpg); epoll_ctl(this->epoll_fd, SIGINT, EV_ADD, EVFILT_SIGNAL);
      epoll_ctl(this->epoll_fd, SIGKILL, EV_ADD, EVFILT_SIGNAL); epoll_ctl(this->epoll_fd, SIGTERM, EV_ADD, EVFILT_SIGNAL);
      struct timespec timeout; memset(&timeout, 0, sizeof(timeout)); timeout.tv_nsec = 10000;
#endif
      // Main loop.
      epoll_event* kevents; ROG* ro;
      do {
        if (RES_TP > t) {
          loop_timer.tick(); t = RES_TP + std::chrono::milliseconds(3);
        }
#if __linux__ || _WIN32
        this->n_events = epoll_wait(this->epoll_fd, this->kevents, RESmaxEVENTS, 1);
#elif __APPLE__
        // kevent is already listening to quit signals.
        this->n_events = kevent(this->epoll_fd, NULL, 0, this->kevents, RESmaxEVENTS, &timeout);
#endif
        if (this->n_events == 0) {
#if !_WIN32
          usleep(10000);
#endif
        } else {
          for (i = 0; i < this->n_events; ++i) {
            kevents = &this->kevents[i];
#if __APPLE__
            this->event_flags = kevents->flags; this->event_fd = kevents->ident;
            if (kevents->filter == EVFILT_SIGNAL) {
              if (event_fd == SIGINT) std::cout << "SIGINT" << std::endl; if (event_fd == SIGTERM) std::cout << "SIGTERM" << std::endl;
              if (this->event_fd == SIGKILL) std::cout << "SIGKILL" << std::endl; RESquit_signal_catched = false; break;
            }
            ro = static_cast<ROG*>(kevents->udata);
#else
            this->event_flags = kevents->events; ro = static_cast<ROG*>(kevents->data.ptr); this->event_fd = ro->$;
#endif
            // Handle errors on sockets.
#if __linux__ || _WIN32
            if (_unlikely(this->event_flags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))) {
#elif __APPLE__
            if (_unlikely(this->event_flags & EV_ERROR)) {
#endif
              //if (this->event_fd != listen_fd) {
#if __cplusplus < _cpp20_date
              if (ro->_) ro->_ = ro->_.resume_with(std::move([](co&& sink) { throw fiber_exception(std::move(sink), ""); return std::move(sink); }));
#else
              loop_timer.cancel(ro->t_id); ro->hrt = 0; fc::Task<ROG> v = std::move(ro->_); epoll_del(event_fd); if (v) v.operator()();
#endif
              //} else {
              //  std::cout << "FATAL ERROR: Error on server socket " << this->event_fd << std::endl; RESquit_signal_catched = false;
              //}
            }
            // Handle new connections.
            else if (listen_fd == this->event_fd) {
              // ACCEPT INCOMMING CONNECTION
              do {
                socket_type socket_fd = accept(listen_fd, this->in_addr, &this->in_len);
                // Subscribe epoll to the socket file descriptor. 将epoll订阅到套接字文件描述符。
#ifndef _WIN32
                if (socket_fd == -1) { break; } if (-1 == ::fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK)) continue;
#else
                if (socket_fd == INVALID_SOCKET) { break; } if (ioctlsocket(socket_fd, FIONBIO, &RESiMode) != NO_ERROR) continue;
#endif
// #ifdef WIN32
//                 RESin_kavars.onoff = k_A[0]; RESin_kavars.keepalivetime = k_A[1] * 1000; RESin_kavars.keepaliveinterval = k_A[2] * 1000;
//                 if (WSAIoctl(socket_fd, SIO_KEEPALIVE_VALS, (LPVOID)&RESin_kavars, RESl_k, (LPVOID)&RESout_kavars, RESl_k, &RESuBR, NULL, NULL) == SOCKET_ERROR) {
//                   DEBUG("WSAIoctl() SIO_KEEPALIVE_VALS error. [%d]", WSAGetLastError()); epoll_del(socket_fd); close_socket(socket_fd); break;
//                 }
// #else
//                 if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &RESkeep_AI, sizeof(RESkeep_AI)) != 0) { epoll_del(socket_fd); close_socket(socket_fd); break; }
//                 setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&k_A[0], sizeof(int)); setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, (void*)&k_A[1], sizeof(int));
//                 setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, (void*)&k_A[2], sizeof(int));
// #endif
                ROG* fib = new ROG{ socket_fd }; fib->_.box = std::unique_ptr<fc::ROG>(fib);// Dark magic
#if __linux__
                epoll_ctl(this->epoll_fd, socket_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, fib);
#elif _WIN32
                epoll_ctl(this->epoll_fd, socket_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLRDHUP, fib);
#elif __APPLE__
                epoll_ctl(this->epoll_fd, socket_fd, EV_ADD, EVFILT_READ | EVFILT_WRITE, fib);
#endif
                // Spawn a new co to handle the connection.继续处理，延续之前未处理的
#if __cplusplus < _cpp20_date
                fib->t_id = this->loop_timer.add_s(k_a - 2, [fib] { if (fib->_) { fib->_.operator()(); } });
                fib->_ = ctx::callcc([this, socket_fd, k_a, &handler, fib, ap](co&& sink) {
                  fib->_ = std::move(sink); Conn c(socket_fd, *this->in_addr, k_a, this->loop_timer, fib, this->epoll_fd);
                  try {
#if _OPENSSL
                    if (this->ssl_ctx && !c.ssl_handshake(this->ssl_ctx)) {
                      loop_timer.cancel(fib->t_id); epoll_del(socket_fd); /*std::cerr << "Error!";*/ return std::move(fib->_);
                    }
#endif
                    handler(c, ap, this); loop_timer.cancel(fib->t_id); epoll_del(socket_fd);
                  } catch (fiber_exception& ex) {
                    loop_timer.cancel(fib->t_id); epoll_del(socket_fd); return std::move(ex.c);
                  } catch (const std::exception&) {
                    loop_timer.cancel(fib->t_id); epoll_del(socket_fd);// std::cerr << "Err: " << e.what() << '\n';
                    return std::move(fib->_);
                  }
                  return std::move(fib->_);
                  });
#else
                fib->t_id = this->loop_timer.add_s(k_a - 2, [fib] { if (fib->_) { fib->_(); } });
                fib->_ = handler(socket_fd, *this->in_addr, k_a, this->loop_timer, fib, this->epoll_fd, ap, this);
#endif
              } while (1);
            } else if (ro->_)ro->_.operator()();// Data available on existing sockets. Wake up the fiber associated with event_fd.
          }
        }
      } while (RESquit_signal_catched);
#if _WIN32
      epoll_close(this->epoll_fd);
#else
      close(this->epoll_fd);
#endif
      free(this->kevents); std::cout << "@";
    }
  };
  static void shutdown_handler(int sig) { RESquit_signal_catched = 0; }
  static void start_server(std::thread& date_thread, socket_type sfd, int n, std::function<_CTX_FUNC> conn_handler, int* k_a, void* ap,
    std::string ssl_key_path = "", std::string ssl_cert_path = "", std::string ssl_ciphers = "") { // Start the winsock DLL
    time(&RES_TIME_T); RES_NOW = localtime(&RES_TIME_T); RES_NOW->tm_isdst = 0; int k_A = k_a[0] + k_a[1] * k_a[2]; if (k_A < 4)k_A = 4;
    RESmaxEVENTS = n > 32 ? n + 32 : n > 7 ? (n << 1) - (n >> 1) : (((n + 1) * (n + 1)) >> 1) + 0x16;
    for (int i = 0; i < n; ++i) {
      RESfus.emplace(std::async(std::launch::async, [i, sfd, &k_a, &k_A, &conn_handler, &n, &ssl_key_path, &ssl_cert_path, &ssl_ciphers, ap] {
        Reactor reactor;
#if _OPENSSL
        if (ssl_cert_path.size()) reactor.ssl_ctx = std::unique_ptr<ssl_context>(new ssl_context{ ssl_key_path, ssl_cert_path, ssl_ciphers });// Initialize the SSL/TLS context.
#endif
        reactor.event_loop(sfd, conn_handler, n, k_A, k_a, i, ap);
        }));
    }
    date_thread.join(); close_socket(sfd);
#if _WIN32
    WSACleanup();
#endif
    std::cout << std::endl;
  }
}
#endif
