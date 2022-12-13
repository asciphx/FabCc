#ifndef REQRES_HH
#define REQRES_HH
#include <functional>
#include <string>
#include "tp/zlib.h"
#include "tp/ctx.hh"
#include <buf.hh>
#include <ctx.hh>
#include <conn.hh>
#include <h/common.h>
#include <hpp/optional.hpp>
#include <file_sptr.hh>
#include <iostream>
#if defined(_MSC_VER)
#include <io.h>
#else
#include <fcntl.h>
#endif
#define BUF_SIZE 0x10000
#define BUF_MAXSIZE 256000000
namespace fc {
  struct App;
  class Req {
	HTTP method;
  public:
	Req();
    Req(fc::Ctx& Ctx) : Ctx(Ctx), fiber(Ctx.fiber), headers(Ctx.headers) {}
    std::string_view header(const char* k) const;
    std::string_view cookie(const char* k) const;
    std::string ip_address() const;

	//Req(HTTP method, fc::Buf url, fc::Buf params, str_map headers, fc::Buf body);
	std::string url;
	//fc::Buf params;
	//fc::Buf body;
	//uint64_t uuid;
	std::unordered_map<std::string_view, std::string_view>& headers;
	fc::Buf ip_addr;
	fc::Ctx& Ctx;
	Conn& fiber;
  };// request

  class Res {
	friend class fc::Conn;
	friend struct fc::App;
	enum algorithm { // 15 is the default value for deflate
	  DEFLATE = 15, // windowBits can also be greater than 15 for optional gzip encoding.
	  // Add 16 to windowBits to write a simple gzip header and trailer around the compressed data instead of a zlib wrapper.
	  GZIP = 15 | 16,
	};
	fc::Buf zlib_cp_str;
	char buffer[8192];
	z_stream stream{};
	std::string path_;
	// fc::Timer timer_;
	int is_file{ 0 };
	long file_size = 0;
    fc::Ctx& Ctx;
	inline void set_header(std::string_view k, std::string_view v) { Ctx.set_header(k, v); }
	inline void set_cookie(std::string_view k, std::string_view v) { Ctx.set_cookie(k, v); }
  public:
	inline Res(fc::Ctx& ctx) : Ctx(ctx) {}
	uint16_t code{ 200 };// Check whether the response has a static file defined.
	std::shared_ptr<file_sptr> __;
	fc::Buf body;
	//void set_header(const fc::Buf& key, fc::Buf value);
	//void add_header(const fc::Buf& key, fc::Buf value);
	void write(const std::string& body_part);
	void write(const std::string_view& body_part);
	void write(const fc::Buf& body_part);
	void write(const char* body_part);
	inline void set_status(int s) { Ctx.set_status(s); }
	fc::Buf& compress_str(char* const str, size_t len);
	fc::Buf& decompress_str(char* const str, size_t len);
  };// response
}

#endif // REQRES_HH
