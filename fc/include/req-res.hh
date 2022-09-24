#ifndef REQRES_HH
#define REQRES_HH
#include <functional>
#include <string>
#include <timer.hh>
#include "tp/zlib.h"
#include <buf.hh>
#include <h/common.h>
#include <file_sptr.hh>
#define BUF_SIZE 0x10000
#define BUF_MAXSIZE 256000000
namespace fc {
  class Conn; class Tcp; struct App;
  class Req {
	friend class fc::Tcp;
	HTTP method;
  public:
	Req();
	Req(HTTP method, fc::Buf url, fc::Buf params, str_map headers, fc::Buf body);
	fc::Buf url;
	fc::Buf params;
	fc::Buf body;
	uint64_t uuid;
	str_map headers;
	std::string ip_addr;
	void add_header(fc::Buf key, fc::Buf value);
  };// request

  class Res {
	friend class fc::Conn;
	friend struct fc::App;
	friend class fc::Tcp;
	enum algorithm { // 15 is the default value for deflate
	  DEFLATE = 15, // windowBits can also be greater than 15 for optional gzip encoding.
	  // Add 16 to windowBits to write a simple gzip header and trailer around the compressed data instead of a zlib wrapper.
	  GZIP = 15 | 16,
	};
	fc::Buf zlib_cp_str;
	char buffer[8192];
	z_stream stream{};
	str_map headers;
	std::string path_;
	fc::Timer timer_;
	int is_file{ 0 };
	long file_size = 0;
	Res();
  public:
	uint16_t code{ 200 };// Check whether the response has a static file defined.
	std::shared_ptr<file_sptr> __;
	fc::Buf body;
	void set_header(const fc::Buf& key, fc::Buf value);
	void add_header(const fc::Buf& key, fc::Buf value);
	const fc::Buf& get_header(const fc::Buf& key);
	void write(const std::string& body_part);
	void write(const fc::Buf& body_part);
	void write(const char* body_part);
	fc::Buf& compress_str(char* const str, size_t len);
	fc::Buf& decompress_str(char* const str, size_t len);
  };// response
}

#endif // REQRES_HH
