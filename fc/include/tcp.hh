#ifndef TCP_HH
#define TCP_HH
#include <uv.h>
#include <string>
#include <map>
#include <thread>
#include <stdio.h>
#include <atomic>
#include <future>
#include <fstream>
#include <timer.hh>
#include <conn.hh>
#include <h/common.h>
#include <app.hh>
#include <directory.hh>

namespace fc {
  static uv_shutdown_t RES_SHUT_REQ; static uv_mutex_t RES_MUTEX;
  class Tcp {
	friend Conn;
	uv_tcp_t _;
	uv_loop_t* loop_;
	sockaddr_storage addr_;
	int max_conn = 0xffff;
	unsigned char max_thread = 1;//not use
	int port_ = DEFAULT_PORT;
	int addr_len;
	int connection_num = 0;
	unsigned short keep_milliseconds = 6000;
	bool opened;
	bool is_ipv6;
	bool init();
	bool bind(const char* ip_addr, int port, bool is_ipv4 = true);
	App* app_;
	bool not_set_types = true;
  public:
	Tcp(App* app = nullptr, uv_loop_t* loop = uv_default_loop());
	virtual ~Tcp();
	bool Start(const char* ip_addr, int port = 0, bool is_ipv4 = true);
	Tcp& setTcpNoDelay(bool enable);
	//If it is within 300, it is seconds; otherwise, it is milliseconds. If less than 1600, multiply by 6
	// from https://man7.org/linux/man-pages/man7/tcp.7.html#TCP_USER_TIMEOUT
	// from https://docs.microsoft.com/en-us/windows/win32/winsock/ipproto-tcp-socket-options#TCP_MAXRT
	// from https://opensource.apple.com/source/xnu/xnu-4570.41.2/bsd/netinet/tcp.h.auto.html#TCP_RXT_CONNDROPTIME
	Tcp& timeout(unsigned short milliseconds = 6000);
	//Set static directory
	Tcp& directory(std::string path);
	//Set upload path
	Tcp& upload_path(std::string path);
	//Set mime types default { "html","htm","ico","css","js","json","svg","png","jpg","gif","txt" }
	Tcp& file_type(const std::vector<std::string_view>& line);
	//Set max connection num
	Tcp& maxConnection(int backlog = SOMAXCONN);
	Tcp& thread(unsigned char n = std::thread::hardware_concurrency());//not use
	Tcp& router(App& app);
	void exit();
	std::unordered_map<std::string_view, std::string_view> content_types;
  protected:
	static void read_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* b);
	static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* b);
	static void write_cb(uv_write_t* wr, int status);
	static void on_close(uv_handle_t* handle);
	static void on_conn(uv_stream_t* server, int status);
	static void on_read(uv_fs_t* req);
  };
}
#endif
