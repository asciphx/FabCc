#ifndef CONN_HH
#define CONN_HH
#include <uv.h>
#include <string>
#include <parser.hh>
#include <req-res.hh>

#define BUF_SIZE 50 * 1024
namespace fc {
  class Conn {
  public:
	unsigned short id;
	Req request_;
	Res response_;
	fc::llParser* parser_;
	uv_write_t _;
	uv_buf_t buf;
	uv_tcp_t* ptr_;
	Conn(fc::llParser* p);
	virtual ~Conn();
  };
}
#endif // CONN_HH
