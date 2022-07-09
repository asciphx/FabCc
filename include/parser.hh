#ifndef PARSER_HH
#define PARSER_HH
#include <string>
#include <unordered_map>
#include <algorithm>
#include <llhttp.h>
#include <h/common.h>
// from https://github.com/ipkn/crow/blob/master/include/crow/parser.h
namespace fc {
  struct llParser: public llhttp__internal_s {
	llParser();
	void process_header();
	void process_message();
	bool check_version(unsigned char major, unsigned char minor) const;
	void set_type(llhttp_type t = llhttp_type::HTTP_REQUEST);
	std::string raw_url;
	std::string url;
	char header_building_state;
	std::string header_field;
	std::string header_value;
	str_map headers;
	//fc::query_string url_params;
	std::string body;
	bool keep_alive, close_conn;
	bool ready = true;
	const static llhttp_settings_s _;
  };
}
#endif // PARSER_HH
