#include <string>
#include <unordered_map>
#include <algorithm>
#include <llhttp.h>
#include <h/common.h>
#include <parser.hh>
// from https://github.com/ipkn/crow/blob/master/include/crow/parser.h
namespace fc {
  static int on_message_begin(llhttp__internal_s* _) {
	llParser* $ = static_cast<llParser*>(_); $->url.clear(); $->raw_url.clear();// $->url_params.clear();
	$->header_field.clear(); $->header_value.clear(); $->headers.clear(); $->body.clear();
	$->header_building_state = $->http_major = $->http_minor = 0; return 0;
  }
  static int on_url(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); $->raw_url.insert($->raw_url.end(), c, c + l); return 0;
  }
  static int on_header_field(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); switch ($->header_building_state) {
	case 0: if (!$->header_value.empty()) $->headers.emplace($->header_field, $->header_value);
	  $->header_field.assign(c, c + l), $->header_building_state = 1; break;
	case 1: $->header_field.insert($->header_field.end(), c, c + l);  break;
	} return 0;
  }
  static int on_header_value(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); switch ($->header_building_state) {
	case 0: $->header_value.insert($->header_value.end(), c, c + l); break;
	case 1: $->header_building_state = 0; $->header_value.assign(c, c + l); break;
	} return 0;
  }
  static int on_headers_complete(llhttp__internal_s* _) {
	llParser* $ = static_cast<llParser*>(_);
	if (!$->header_field.empty()) $->headers.emplace($->header_field, $->header_value);
	$->keep_alive = ($->http_major == 1 && $->http_minor == 0) ?
	  (($->flags & F_CONNECTION_KEEP_ALIVE) ? true : false) :
	  (($->http_major == 1 && $->http_minor == 1) ? true : false);
	$->process_header(); return 0;//llhttp_should_keep_alive(_);
  }
  static int on_body(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); $->body.insert($->body.end(), c, c + l); return 0;
  }
  static int on_message_complete(llhttp__internal_s* _) {
	llParser* $ = static_cast<llParser*>(_); $->url = $->raw_url.substr(0, $->raw_url.find('?'));
	$->process_message(); $->ready = true; return 0;//$->url_params = query_string($->raw_url);
  }
  const llhttp_settings_s llParser::_ = {
	  on_message_begin,
	  on_url,
	  nullptr,
	  on_header_field,
	  on_header_value,
	  on_headers_complete,
	  on_body,
	  on_message_complete
  };
  void llParser::process_header() {
	//handler_->handle_header();
  }
  void llParser::process_message() {
	//handler_->handle();
  }
  void llParser::set_type(llhttp_type t) { this->type = t; }
  //Req to_request() const {
  //  return Req{ static_cast<HTTP>(method), std::move(raw_url), std::move(url), std::move(url_params),
  //  std::move(headers), std::move(body) };
  //}
  bool llParser::check_version(unsigned char major, unsigned char minor) const {
	return http_major == major && http_minor == minor;
  }
  llParser::llParser() { llhttp__internal_init(this); this->type = HTTP_REQUEST; this->settings = (void*)&_; }
}
