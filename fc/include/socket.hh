#pragma once
#ifdef ENABLE_SSL
#include <boost/asio/ssl.hpp>
#endif
#if defined _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <mstcpip.h>
typedef UINT_PTR socket_type;//SD_RECEIVE，SD_SEND，SD_BOTH
static unsigned int RES_RCV = 5000;
static unsigned int RES_SED = 10000;
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
typedef int socket_type;
static struct timeval RES_RCV { 5, 0 };//max{5,0},read
static struct timeval RES_SED { 10, 0 };//write
#endif
namespace fc {
  static int RES_KEEP_Alive = 1;//开启keepalive
  enum send_type { S_READ, S_WRITE, S_BOTH };
  class Socket {
    Socket& operator=(const Socket&) = delete;
    Socket(const Socket&) = delete;
  public:
    socket_type id;
    Socket(unsigned short milliseconds) noexcept;
    bool reading_ = false;
    bool write(const char* buf, int size);
    int read(char* buf, int max_size);
    int shut(send_type type);
    static int shut(socket_type fd, send_type d);
    int close_fd(socket_type fd);
    // idle:首次发送报文的等待时间,intvl:保持发送报文的间隔,probes: 报文侦测间隔次数
    // keep-alive time seconds = idle + intvl * probes
    int set_keep_alive(socket_type& fd, int idle, int intvl = 1, unsigned char probes = 10);
    bool is_open();
    void close();
  };
}
