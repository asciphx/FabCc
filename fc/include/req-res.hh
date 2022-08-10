#ifndef REQRES_HH
#define REQRES_HH
#include <functional>
#include <string>
#include <timer.hh>
#include "tp/zlib.h"
#include <buffer.hh>
#include <h/common.h>
#define BUF_SIZE 0x28000
#define BUF_MAXSIZE 256000000
namespace fc {
  class Conn; class Tcp; struct App;
  class Req {
	friend class fc::Tcp;
	HTTP method;
  public:
	Req();
	Req(HTTP method, fc::Buffer url, fc::Buffer params, str_map headers, fc::Buffer body);
	fc::Buffer url;
	fc::Buffer params;
	fc::Buffer body;
	uint64_t uuid;
	str_map headers;
	std::string ip_addr;
	void add_header(std::string key, std::string value);
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
	fc::Buffer zlib_cp_str;
	char buffer[8192];
	z_stream stream{};
	str_map headers;
	std::string path_;
	fc::Timer timer_;
	int is_file{ 0 };
	long file_size = 0;
	std::function<void(int64_t o, int64_t k,std::function<void(const char* c, size_t s, std::function<void()> d)> sink)> provider;
	Res();
  public:
	uint16_t code{ 200 };// Check whether the response has a static file defined.
	fc::Buffer body;
	void set_header(const std::string& key, std::string value);
	void add_header(const std::string& key, std::string value);
	const std::string& get_header(const std::string& key);
	void write(const std::string& body_part);
	void write(const fc::Buffer& body_part);
	fc::Buffer& compress_str(char* const str, size_t len);
	fc::Buffer& decompress_str(char* const str, size_t len);
  };// response
}

#endif // REQRES_HH
