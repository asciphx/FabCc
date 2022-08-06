#include <parser.hh>
// from https://github.com/ipkn/crow/blob/master/include/crow/parser.h
namespace fc {
  static int on_message_begin(llhttp__internal_s* _) {
	llParser* $ = static_cast<llParser*>(_); $->url.reset(); $->url_params.reset();
	$->headers.clear(); $->body.reset(); $->http_major = $->http_minor = 0; return 0;
  }
  static int on_url(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); $->url_params = DecodeURL(c, l);  return 0;
  }
  static int on_header_field(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); $->header_field.assign(c, c + l); return 0;
  }
  static int on_header_value(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); $->header_value.assign(c, c + l);
	$->headers.emplace($->header_field.b2s(), $->header_value.b2s());
	$->header_field.reset(); $->header_value.reset(); return 0;
  }
  static int on_headers_complete(llhttp__internal_s* _) {
	llParser* $ = static_cast<llParser*>(_);
	//$->keep_alive = ($->http_major == 1 && $->http_minor == 0) ?
	//  (($->flags & F_CONNECTION_KEEP_ALIVE) ? true : false) :
	//  (($->http_major == 1 && $->http_minor == 1) ? true : false);
	return 0;//llhttp_should_keep_alive(_);
  }
  static int on_body(llhttp__internal_s* _, const char* c, size_t l) {
	llParser* $ = static_cast<llParser*>(_); $->body.insert($->body.end_, c, c + l); return 0;
  }
  static int on_message_complete(llhttp__internal_s* _) {
	llParser* $ = static_cast<llParser*>(_); unsigned int l = $->url_params.find('?');
	if (l != -1) {
	  $->url = $->url_params.subbuf(0, l); $->url_params = $->url_params.subbuf(++l);
	} else { $->url << $->url_params; }
	$->ready = true; return 0;//$->url_params = query_string($->url_params); handler_->handle();
  }
  Req llParser::to_request() const {
	return Req{ static_cast<HTTP>(method), std::move(url), std::move(url_params), std::move(headers), std::move(body) };
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
  void llParser::set_type(llhttp_type t) { this->type = t; }
  llParser::llParser(): header_field(0x1f), header_value(0x7f), body(0x2ff), url_params(0x3f), url(0x1f) {
	llhttp__internal_init(this); this->type = HTTP_REQUEST; this->settings = (void*)&_;
  }
}
