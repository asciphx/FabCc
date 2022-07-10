#ifndef CONN_HH
#define CONN_HH
#include <uv.h>
#include <string>
#include <parser.hh>
#include <req-res.hh>
#include <app.hh>

#define BUF_SIZE 500 * 1024
namespace fc {
  class Conn {
  public:
	uv_write_t _;
	unsigned short id;
	Req req_;
	Res res_;
	App* app_;
	Conn();
	uv_buf_t rbuf;
	uv_tcp_t ptr_;
	std::string buf_;
	fc::llParser parser_;
	const char* status_ = "404 Not Found\r\n";
	virtual ~Conn();
	bool ready = true;
  };
}
#endif // CONN_HH
