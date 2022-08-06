#ifndef PARSER_HH
#define PARSER_HH
#include <unordered_map>
#include <algorithm>
#include <h/llhttp.h>
#include <h/common.h>
#include <req-res.hh>
#include <buffer.hh>
// from https://github.com/ipkn/crow/blob/master/include/crow/parser.h
namespace fc {
  struct llParser: public llhttp__internal_s {
	llParser();
	void set_type(llhttp_type t = llhttp_type::HTTP_REQUEST);
	fc::Buffer url_params;
	fc::Buffer url;
	fc::Buffer body;
	fc::Buffer header_field;
	fc::Buffer header_value;
	fc::str_map headers;
	//fc::query_string url_params;
	bool ready = true/*, keep_alive*/;
	const static llhttp_settings_s _;
	Req to_request() const;
  };
}
#endif // PARSER_HH
