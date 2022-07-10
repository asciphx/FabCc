#include <uv.h>
#include <string>
#include <map>
#include <thread>
#include <stdio.h>
#include <atomic>
#include <future>
#include <chrono>
#include <conn.hh>
#include <parser.hh>
#include <h/common.h>
#include <h/config.h>
#include <buffer.hh>
static std::atomic<unsigned char> RES_IDEX = 0xff;
namespace fc {
  class Tcp {
	friend Conn;
	uv_tcp_t _;
	uv_loop_t* loop_;
	sockaddr_storage addr_;
	unsigned char num = 1;//待实现控制线程数量
	int port_ = 8080;//默认端口
	int addr_len;
	bool opened;
	bool is_ipv6;
	bool init();
	bool bind(const char* ip_addr, int port, bool is_ipv4 = true);
	bool listen(int backlog = 0xffff);//SOMAXCONN
	static void set_status(Conn* co, uint16_t status);
	App* app_;
  public:
	Tcp(App* app, uv_loop_t* loop = uv_default_loop());
	virtual ~Tcp();
	bool Start(const char* ip_addr, int port = 0, bool is_ipv4 = true);//默认ipv4
	Tcp& setTcpNoDelay(bool enable);
	Tcp& thread(unsigned char n = std::thread::hardware_concurrency());
	void exit();
	void close();//待实现
  protected:
	static void read_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* b);
	static void write_cb(uv_write_t* req, int status);//
	static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* b);
	static void on_exit(uv_handle_t* handle);
	static void on_close(uv_handle_t* handle);
	static void on_connection(uv_stream_t* server, int status);
  };
}