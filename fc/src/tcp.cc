#include "tcp.hh"
#include "app.hh"
#include <h/common.h>
#include <http_error.hh>
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
      //addr.sin_addr.s_addr;
      inet_pton(AF_INET, ip, (void*)&addr.sin_addr);
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
    assert(fd >= 0 && fd < fd_to_fiber_idx.size()); socket_type fiber_idx = fd_to_fiber_idx[fd];
    assert(fiber_idx >= 0 && fiber_idx < R_fibers.size()); return R_fibers[fiber_idx];
  }
  void Reactor::reassign_fd_to_fiber(socket_type fd, socket_type fiber_idx) {
    fd_to_fiber_idx[fd] = fiber_idx;
  }
  void Reactor::epoll_ctl(epoll_handle_t epoll_fd, socket_type fd, int action, socket_type flags) {
#if __linux__ || _WIN32
    epoll_event event; memset(&event, 0, sizeof(event)); event.data.fd = fd; event.events = flags;
    if (-1 == ::epoll_ctl(epoll_fd, action, fd, &event) && errno != EEXIST)
      std::cout << "epoll_ctl error: " << strerror(errno) << std::endl;
#elif __APPLE__
    struct kevent ev_set; EV_SET(&ev_set, fd, flags, action, 0, 0, NULL);
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
    if (fd_to_fiber_idx.size() < new_fd + 1) fd_to_fiber_idx.resize(new_fd * 2 + 2, -1);
    fd_to_fiber_idx[new_fd] = fiber_idx;
  }
  void Reactor::epoll_mod(socket_type fd, int flags) {
#if __linux__ || _WIN32
    epoll_ctl(epoll_fd, fd, EPOLL_CTL_MOD, flags);
#elif __APPLE__
    epoll_ctl(epoll_fd, fd, 2, flags);
#endif
  }
}
