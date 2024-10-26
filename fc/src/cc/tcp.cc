#include "hh/tcp.hh"
#include "app.hh"
#include "h/common.h"
#include "hh/http_error.hh"
namespace fc {
#if defined WIN32 && !defined(_MSVC_LANG)
  void inet_pton(int af, const char* src, void* dst) {
    struct sockaddr_in ip4; struct sockaddr_in6 ip6; char ipaddr[64]; sprintf(ipaddr, "%s:2020", src);
    int as = -1; if (af == AF_INET) {
      as = sizeof(SOCKADDR_IN); WSAStringToAddressA((LPSTR)ipaddr, af, NULL, (LPSOCKADDR)&ip4, &as); memcpy(dst, &(ip4.sin_addr), 4);
    } else if (af == AF_INET6) {
      as = sizeof(SOCKADDR_IN6); WSAStringToAddressA((LPSTR)ipaddr, af, NULL, (LPSOCKADDR)&ip6, &as); memcpy(dst, &(ip6.sin6_addr), 16);
    }
  }
#endif
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
#ifdef WIN32
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&RESon, sizeof(int));
        setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&RESon, sizeof(int));
        setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&RESrcv, sizeof(int));
        setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&RESsed, sizeof(int));
#else
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &RESon, sizeof(int));
        setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &RESon, sizeof(int));
        setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &RESrcv, sizeof(int));
        setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, &RESsed, sizeof(int));
#endif
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
      struct sockaddr_in addr { AF_INET, htons(port) };
      inet_pton(AF_INET, ip, (void*)&addr.sin_addr);
#ifdef WIN32
      setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&RESon, sizeof(int));
      setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&RESon, sizeof(int));
      setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&RESrcv, sizeof(int));
      setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&RESsed, sizeof(int));
#else
      setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &RESon, sizeof(int));
      setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &RESon, sizeof(int));
      setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &RESrcv, sizeof(int));
      setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, &RESsed, sizeof(int));
#endif
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
  void Reactor::epoll_ctl(epoll_handle_t epoll_fd, socket_type fd, int action, socket_type flags, void* ptr) {
#if __linux__ || _WIN32
    epoll_event e{ static_cast<uint32_t>(flags) }; e.data.ptr = ptr;
    if (-1 == ::epoll_ctl(epoll_fd, action, fd, &e) && errno != EEXIST) std::cout << "epoll_ctl error: " << strerror(errno) << std::endl;
#elif __APPLE__
    struct kevent e; e.udata = ptr; EV_SET(&e, fd, flags, action, 0, 0, NULL); kevent(epoll_fd, &e, 1, NULL, 0, NULL);
#endif
  };
}
