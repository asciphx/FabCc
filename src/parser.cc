#include <string>
#include <unordered_map>
#include <algorithm>
#include <llhttp.h>
#include <h/common.h>
#include <parser.hh>
// from https://github.com/ipkn/crow/blob/master/include/crow/parser.h
namespace fc {
  static int on_message_begin(llhttp__internal_s* self_) {
	llParser* $ = static_cast<llParser*>(self_); $->url.clear(); $->raw_url.clear();
	$->header_field.clear(); $->header_value.clear(); $->headers.clear();
	$->body.clear();// $->url_params.clear();
	$->header_building_state = $->http_major = $->http_minor = 0; return 0;
  }
  static int on_url(llhttp__internal_s* self_, const char* at, size_t length) {
	llParser* $ = static_cast<llParser*>(self_); $->raw_url.insert($->raw_url.end(), at, at + length); return 0;
  }
  static int on_header_field(llhttp__internal_s* self_, const char* at, size_t length) {
	llParser* $ = static_cast<llParser*>(self_);
	switch ($->header_building_state) {
	case 0: if (!$->header_value.empty()) $->headers.emplace($->header_field, $->header_value);
	  $->header_field.assign(at, at + length), $->header_building_state = 1; break;
	case 1: $->header_field.insert($->header_field.end(), at, at + length);  break;
	}
	return 0;
  }
  static int on_header_value(llhttp__internal_s* self_, const char* at, size_t length) {
	llParser* $ = static_cast<llParser*>(self_);
	switch ($->header_building_state) {
	case 0: $->header_value.insert($->header_value.end(), at, at + length); break;
	case 1: $->header_building_state = 0; $->header_value.assign(at, at + length); break;
	}
	return 0;
  }
  static int on_headers_complete(llhttp__internal_s* self_) {
	llParser* $ = static_cast<llParser*>(self_);
	if (!$->header_field.empty())
	  $->headers.emplace(std::move($->header_field), std::move($->header_value));
	$->keep_alive = ($->http_major == 1 && $->http_minor == 0) ?
	  (($->flags & F_CONNECTION_KEEP_ALIVE) ? true : false) :
	  (($->http_major == 1 && $->http_minor == 1) ? true : false);
	$->process_header(); return 0;//llhttp_should_keep_alive(self_);
  }
  static int on_body(llhttp__internal_s* self_, const char* at, size_t length) {
	llParser* $ = static_cast<llParser*>(self_);
	$->body.insert($->body.end(), at, at + length); return 0;
  }
  static int on_message_complete(llhttp__internal_s* self_) {
	llParser* $ = static_cast<llParser*>(self_);
	$->url = $->raw_url.substr(0, $->raw_url.find('?'));
	//$->url_params = query_string($->raw_url);
	$->process_message(); $->ready = true; return 0;
  }
  void llParser::process_header() {
	//handler_->handle_header();
  }
  void llParser::process_message() {
	//handler_->handle();
  }
  //Conn to_request() const {
  //  return Conn{ static_cast<HTTP>(method), std::move(raw_url), std::move(url), std::move(url_params),
  //  std::move(headers), std::move(body) };
  //}
  bool llParser::check_version(unsigned char major, unsigned char minor) const {
	return http_major == major && http_minor == minor;
  }
  llParser::llParser() { llhttp__internal_init(this); this->type = HTTP_REQUEST; this->settings = (void*)&_; }
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
}
