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

namespace fc {
  static unsigned char RES_PASER_IDEX = 0xff;
  static fc::llParser parser_[256];
  class Tcp {
	friend Conn;
	uv_tcp_t _;
	uv_loop_t* loop_;
	sockaddr_storage addr_;
	unsigned char num = 1;
	int port_ = 8080;//默认端口
	int addr_len;
	bool opened;
	bool is_ipv6;
	bool init();
	bool bind(const char* ip_addr, int port, bool is_ipv4 = true);
	bool listen(int backlog = 0xffff);//SOMAXCONN
	_INLINE fc::llParser* getParser() {
	_: fc::llParser* l = &parser_[++RES_PASER_IDEX];
	  if (l->ready) { l->ready = false; return l; } std::this_thread::yield(); goto _;
	}
	static void set_status(Conn* co, uint16_t status);
  public:
	Tcp(uv_loop_t* loop = uv_default_loop());
	virtual ~Tcp();
	bool Start(const char* ip_addr, int port = 0);//默认ipv4
	bool setTcpNoDelay(bool enable);
	void thread(unsigned char n = std::thread::hardware_concurrency());
	void test(); void exit();
	//待实现
	void close();
  protected:
	static void read_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* b);
	static void write_cb(uv_write_t* req, int status);
	static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* b);
	static void on_exit(uv_handle_t* handle);
	static void on_close(uv_handle_t* handle);
	static void on_connection(uv_stream_t* server, int status);
  };
}