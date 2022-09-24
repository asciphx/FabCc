#ifndef CONN_HH
#define CONN_HH
#include <tp/uv.h>
#include <string>
#include <atomic>
#include <parser.hh>
#include <req-res.hh>
#include <h/config.h>
#if defined _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <mstcpip.h>
typedef UINT_PTR socket_type;
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
  enum sd_type { _READ, _WRITE, _BOTH };
  class Conn {
	Conn& operator=(const Conn&) = delete;
	Conn(const Conn&) = delete;
  public:
	uv_write_t _;
	socket_type id;
	Req req_;
	Res res_;
	void* app_;
	void* data;
	unsigned short keep_milliseconds;
	Conn(unsigned short milliseconds, uv_loop_t* l, std::atomic<uint16_t>& queue_length) noexcept;//
	uv_buf_t rbuf;
	uv_loop_t* loop_;
	uv_tcp_t slot_;
	bool reading_ = false;
	char readbuf[0x28000];
	fc::Buf buf_;
	fc::llParser parser_;
	const char* status_ = "404 Not Found\r\n";
	void* tcp_;
	std::atomic<uint16_t>& queue_length_;
	virtual ~Conn();
	bool write(const char* buf, int size, int flag = 0);
	int read(char* buf, int max_size);
	void set_status(Res& res, uint16_t status);
    std::function<void(const char* c, size_t s, std::function<void()> f)> sink_;
	int shut(socket_type fd, sd_type d);
	int close_fd(socket_type fd);
	// idle:首次发送报文的等待时间,intvl:保持发送报文的间隔,probes: 报文侦测间隔次数
	// keep-alive time seconds = idle + intvl * probes
	int set_keep_alive(socket_type& fd, int idle, int intvl = 1, unsigned char probes = 10);
  };
}
#endif // CONN_HH
