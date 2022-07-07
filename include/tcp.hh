#include <uv.h>
#include <string>
#include <map>
#include <thread>
#include <stdio.h>
#include <conn.hh>
#include <parser.hh>
#if _DEBUG
#    define DEBUG printf
#else
#    define DEBUG(...)
#endif
#define TYPE_GET(t, ptr, member) (t*)(ptr)-((size_t)&reinterpret_cast<char const volatile&>(((t*)0)->member))
static unsigned char RES_PASER = 0xff;
namespace fc {
  class Tcp {
	friend Conn;
	uv_tcp_t _;
	uv_loop_t* loop_;
	fc::llParser parser_[256];
	bool opened;
	bool init();
	bool bind(const char* ip_addr, int port, bool is_ip4 = true);
	bool listen(int backlog = SOMAXCONN);
	fc::llParser* getParser() {
	_: fc::llParser* l = &parser_[++RES_PASER];
	  if (l->ready) { l->ready = false; return l; } std::this_thread::yield(); goto _;
	}
  public:
	Tcp(uv_loop_t* loop = uv_default_loop());
	virtual ~Tcp();
	bool Start(const char* ip_addr, int port);//默认ipv4
	bool setTcpNoDelay(bool enable);
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