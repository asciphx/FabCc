#ifndef CONN_HH
#define CONN_HH
#include <uv.h>
#include <string>
#include <parser.hh>
#include <req-res.hh>
#include <app.hh>
#if defined __linux__ || defined __APPLE__
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#if defined _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <mstcpip.h>
#endif

#define BUF_SIZE 500 * 1024
namespace fc {
#if defined _WIN32
  typedef SOCKET socket_type;
#else
  typedef int socket_type;
#endif
  static int RES_KEEP_Alive = 1;//开启keepalive
  //enum sd_type { _READ, _WRITE, _BOTH };
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
	//int shut(socket_type fd, sd_type d);
	//int close_fd(socket_type fd);
	// idle: 保持空闲状态的秒数
	// intvl:  发送报文的间隔时间
	// probes: 报文侦测间隔次数
	int set_keep_alive(socket_type& fd, int idle, int intvl = 1, unsigned char probes = 10);
	void set_status(Res& res, uint16_t status);
  };
}
#endif // CONN_HH
