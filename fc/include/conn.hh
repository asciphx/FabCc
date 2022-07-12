#ifndef CONN_HH
#define CONN_HH
#include <uv.h>
#include <string>
#include <parser.hh>
#include <req-res.hh>
#include <app.hh>
#include <h/config.h>
#if defined __linux__ || defined __APPLE__
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
typedef int socket_type;
static struct timeval RES_RCV { 3, 0 };//max{5,0},read
static struct timeval RES_SED { 10, 0 };//write
#endif
#if defined _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <mstcpip.h>
typedef SOCKET socket_type;
static unsigned int RES_RCV = 3000;
static unsigned int RES_SED = 10000;
#endif
#define BUF_SIZE 0x10000
namespace fc {
  static int RES_KEEP_Alive = 1;//开启keepalive
  enum sd_type { _READ, _WRITE, _BOTH };
  class Conn {
  public:
	uv_write_t _;
	socket_type id;
	Req req_;
	Res res_;
	App* app_;
	Conn();
	uv_buf_t rbuf, wbuf;
	uv_tcp_t slot_;
	std::string buf_;
	fc::llParser parser_;
	const char* status_ = "404 Not Found\r\n";
	virtual ~Conn();
	int shut(socket_type fd, sd_type d);
	//int close_fd(socket_type fd);
	// idle:   首次发送报文的等待时间
	// intvl:  保持发送报文的间隔
	// probes: 报文侦测间隔次数
	// keep-alive time seconds = idle + intvl * probes
	int set_keep_alive(socket_type& fd, int idle, int intvl = 1, unsigned char probes = 10);
	void set_status(Res& res, uint16_t status);
  };
}
#endif // CONN_HH
