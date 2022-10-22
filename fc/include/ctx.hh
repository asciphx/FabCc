#ifndef CTX_H
#define CTX_H
#include <cstring>
#include <functional>
#include <iostream>
#include <hpp/string_view.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#endif
#if __linux__ // the sendfile header does not exists on macos.
#include <sys/sendfile.h>
#endif
#include <unordered_map>
#include <lexical_cast.hh>
#include <http_error.hh>
#include <tcp.hh>
#include <h/any_types.h>
#include <http_top_header_builder.hh>
#include <buf.hh>
#include <input_buffer.hh>
#include <output_buffer.hh>
namespace fc {
  static char* date_buf = nullptr; static int date_buf_size = 0;
#ifndef _WIN32
  http_top_header_builder http_top_header [[gnu::weak]];
#else
  __declspec(selectany) http_top_header_builder http_top_header;
#endif
  struct Ctx {
	Ctx(input_buffer& _rb, Conn& _fiber): rb(_rb), fiber(_fiber) {
	  get_parameters_map.reserve(10); response_headers.reserve(20);
	  output_stream = output_buffer(65536, [&](const char* d, int s) { fiber.write(d, s); });
	  headers_stream = output_buffer(998, [&](const char* d, int s) { output_stream << std::string_view(d, s); });
	}
	Ctx& operator=(const Ctx&) = delete;
	Ctx(const Ctx&) = delete;
	std::string_view header(const char* key);
	std::string_view cookie(const char* key);
	std::string_view get_parameter(const char* key);
	std::string& url();
	std::string& method();
	std::string_view http_version();
	void format_top_headers(output_buffer& output_stream);
	void prepare_request();
	void respond(const fc::Buf& s);
	void respond_if_needed();
	void set_header(std::string_view k, std::string_view v);
	void set_cookie(std::string_view k, std::string_view v);
	void set_status(int status);
	// Send a file.
	void send_file(const char* path);
	void add_header_line(const char* l);
	const char* last_header_line();
	// split a string, starting from cur && ending with split_char.
	// Advance cur to the end of the split.
	std::string split(const char*& cur, const char* line_end, char split_char);
	void index_headers();
	void index_cookies();
	void parse_first_line();
	std::string_view get_parameters_string();
	std::string_view read_whole_body();
	// Read post parameters in the body.
	std::unordered_map<std::string_view, std::string_view> post_parameters();
	void prepare_next_request();
	void flush_responses();
	template <typename C> void url_decode_parameters(std::string_view content, C kv_callback) {
	  const char* c = content.data();
	  const char* end = c + content.size();
	  if (c < end) {
		while (c < end) {
		  std::string_view key = split(c, end, '=');
		  std::string_view value = split(c, end, '&');
		  kv_callback(key, value);
		  // printf("kv: '%.*s' -> '%.*s'\n", key.size(), key.data(), value.size(), value.data());
		}
	  }
	}
	template <typename F> void parse_get_parameters(F processor) {
	  url_decode_parameters(get_parameters_string(), processor);
	}
	fc::socket_type socket_fd;
	input_buffer& rb;
	int status_code_ = 200;
	const char* status_ = "200 OK";
	std::string method_;
	std::string url_;
	std::string_view http_version_;
	std::string_view content_type_;
	bool chunked_;
	int content_length_;
	std::unordered_map<std::string_view, std::string_view> header_map;
	std::unordered_map<std::string_view, std::string_view> cookie_map;
	std::vector<std::pair<std::string_view, std::string_view>> response_headers;
	std::unordered_map<std::string_view, std::string_view> get_parameters_map;
	std::unordered_map<std::string_view, std::string_view> post_parameters_map;
	std::string_view get_parameters_string_;
	// std::vector<std::string> strings_saver;
	bool is_body_read_ = false;
	std::string body_local_buffer_;
	std::string_view body_;
	std::string_view body_start;
	const char* body_end_ = nullptr;
	std::vector<const char*> header_lines;
	fc::Conn& fiber;
	output_buffer headers_stream;
	bool response_written_ = false;
	output_buffer output_stream;
  };

} // namespace fc

#endif