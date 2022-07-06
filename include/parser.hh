#pragma once
#include <string>
#include <unordered_map>
#include <algorithm>
#include <llhttp.h>
#include <str_map.hh>
namespace cc {
  enum class HTTP {
	DEL = 0, GET, HEAD, POST, PUT, CONNECT, OPTIONS,
	TRACE, PATCH = 28, PURGE, InternalMethodCount
  };
  template <typename Handler>
  struct LLParser: public llhttp_t {
	static int on_message_begin(llhttp_t* self_);
	static int on_url(llhttp_t* self_, const char* at, size_t length);
	static int on_header_field(llhttp_t* self_, const char* at, size_t length);
	static int on_header_value(llhttp_t* self_, const char* at, size_t length) ;
	static int on_headers_complete(llhttp_t* self_);
	static int on_body(llhttp_t* self_, const char* at, size_t length);
	static int on_message_complete(llhttp_t* self_);
	LLParser(Handler* handler): handler_(handler);
	bool feed(const char* buffer, int length);
	void clear();
	void process_header();
	void process_message();
	void set_connection_parameters();
	bool check_version(unsigned char major, unsigned char minor) const;
	std::string raw_url;
	std::string url;
	int header_building_state = 0;
	std::string header_field;
	std::string header_value;
	str_map headers;
	//cc::query_string url_params;
	std::string body;
	bool keep_alive;
	Handler* handler_;
	constexpr const static llhttp_settings_s settings_ = {
		on_message_begin,
		on_url,
		on_header_field,
		on_header_value,
		on_headers_complete,
		on_body,
		on_message_complete,
	};
  };
}
