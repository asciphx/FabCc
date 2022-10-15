#ifndef PARSER_HH
#define PARSER_HH
#include <unordered_map>
#include <algorithm>
#include <h/llhttp.h>
#include <h/common.h>
#include <req-res.hh>
#include <buf.hh>
namespace fc {
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
	Req to_request() const;
  };
}
#endif // PARSER_HH
