#ifndef PARSER_HH
#define PARSER_HH
#include <unordered_map>
#include <algorithm>
#include <h/llhttp.h>
#include <h/common.h>
#include <hpp/string_view.hpp>
#include <str_map.hh>
#include <buf.hh>
namespace fc {
  class Conn;
  struct llParser: public llhttp__internal_s {
	llParser();
	void set_type(llhttp_type t = llhttp_type::HTTP_REQUEST);
	fc::Buf url_params;
	fc::Buf url;
	fc::Buf body;
	fc::Buf header_field;
	fc::Buf header_value;
	fc::str_map headers;
	//fc::query_string url_params;
	//bool keep_alive;
	const static llhttp_settings_s _;
	template<typename R>
	R to_request(Conn& fib, std::unordered_map<std::string_view, std::string_view>& cookie_map) {
	  return R{ static_cast<HTTP>(method), url, url_params, headers, body, fib, cookie_map };
	}
  };
}
#endif // PARSER_HH
