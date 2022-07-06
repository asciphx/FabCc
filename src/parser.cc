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
	static int on_message_begin(llhttp_t* self_) {
	  LLParser* $ = static_cast<LLParser*>(self_); $->clear(); return 0;
	}
	static int on_url(llhttp_t* self_, const char* at, size_t length) {
	  LLParser* $ = static_cast<LLParser*>(self_); $->raw_url.insert($->raw_url.end(), at, at + length); return 0;
	}
	static int on_header_field(llhttp_t* self_, const char* at, size_t length) {
	  LLParser* $ = static_cast<LLParser*>(self_);
	  switch ($->header_building_state) {
	  case 0:
		if (!$->header_value.empty()) {
		  $->headers.emplace(std::move($->header_field), std::move($->header_value));
		}
		$->header_field.assign(at, at + length);
		$->header_building_state = 1;
		break;
	  case 1:
		$->header_field.insert($->header_field.end(), at, at + length);
		break;
	  }
	  return 0;
	}
	static int on_header_value(llhttp_t* self_, const char* at, size_t length) {
	  LLParser* $ = static_cast<LLParser*>(self_);
	  switch ($->header_building_state) {
	  case 0: $->header_value.insert($->header_value.end(), at, at + length); break;
	  case 1: $->header_building_state = 0; $->header_value.assign(at, at + length); break;
	  }
	  return 0;
	}
	static int on_headers_complete(llhttp_t* self_) {
	  LLParser* $ = static_cast<LLParser*>(self_);
	  if (!$->header_field.empty()) {
		$->headers.emplace(std::move($->header_field), std::move($->header_value));
	  }
	  $->set_connection_parameters(); $->process_header(); return 0;
	}
	static int on_body(llhttp_t* self_, const char* at, size_t length) {
	  LLParser* $ = static_cast<LLParser*>(self_);
	  $->body.insert($->body.end(), at, at + length); return 0;
	}
	static int on_message_complete(llhttp_t* self_) {
	  LLParser* $ = static_cast<LLParser*>(self_);
	  // url params
	  $->url = $->raw_url.substr(0, $->raw_url.find('?'));
	  //$->url_params = query_string($->raw_url);
	  $->process_message(); return 0;
	}
	LLParser(Handler* handler): handler_(handler) {
	  llhttp__internal_init(this); this->type = HTTP_REQUEST; this->settings = &settings_;
	}
	bool feed(const char* buffer, int length) {
	  return llhttp_execute(this, buffer, length) == 0;
	}
	void clear() {
	  url.clear();
	  raw_url.clear();
	  header_field.clear();
	  header_value.clear();
	  headers.clear();
	  body.clear();
	  header_building_state = 0;
	  http_major = 0;
	  http_minor = 0;
	  //url_params.clear();
	}
	void process_header() {
	  //handler_->handle_header();
	}
	void process_message() {
	  //handler_->handle();
	}
	void set_connection_parameters() {
	  keep_alive = (http_major > 0 && http_minor > 0) ? (flags & F_CONNECTION_CLOSE ? false : true) :
		(flags & F_CONNECTION_KEEP_ALIVE ? true : false);
	}
	//Req to_request() const {
	//  return Req{ static_cast<HTTP>(method), std::move(raw_url), std::move(url), std::move(url_params),
	//  std::move(headers), std::move(body) };
	//}
	bool check_version(unsigned char major, unsigned char minor) const {
	  return http_major == major && http_minor == minor;
	}
	std::string raw_url;
	std::string url;
	int header_building_state = 0;
	std::string header_field;
	std::string header_value;
	str_map headers;
	//query_string url_params;
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
