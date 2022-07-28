#include <req-res.hh>
#include <hpp/http_error.hpp>
namespace fc {
  Req::Req() { body.reserve(0x1ff); params.reserve(0x3f); url.reserve(0x1f); };
  Req::Req(HTTP method, std::string url, std::string params, str_map headers, std::string body/*, bool k*/):/* keep_alive(k),*/
	method(method), url(std::move(url)), params(std::move(params)), headers(std::move(headers)), body(std::move(body)) {}
  void Req::add_header(std::string key, std::string value) { headers.emplace(std::move(key), std::move(value)); }
  /******************************** ************************************/
  Res::Res() { body.reserve(0xff); path_.reserve(0x3f); };
  void Res::set_header(const std::string& key, std::string value) { headers.erase(key); headers.emplace(key, std::move(value)); }
  void Res::add_header(const std::string& key, std::string value) { headers.emplace(key, std::move(value)); }
  const std::string& Res::get_header(const std::string& key) {
	if (headers.count(key)) { return headers.find(key)->second; } return RES_empty;
  }
  void Res::write(const std::string& body_part) { body += body_part; };
  void Res::set_static_file_info(std::string path) {
	struct stat statbuf_; path_ = detail::directory_; path_ += path.c_str() + 1;
	is_file = stat(path_.c_str(), &statbuf_);
	if (is_file == 0 /*&& statbuf_.st_size < BUF_SIZE*/) {
	  std::string::iterator i = --path.end(); if (*--i == '.')goto _; if (*--i == '.')goto _;
	  if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
	  if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
	  throw err::not_found();
	_:std::size_t last_dot = i._Ptr - path.begin()._Ptr + 1;
	  if (last_dot) {
		std::string ss = path.substr(last_dot);
		std::string_view extension(ss.data(), ss.size());// printf("<%d,%s>", is_file, path_.c_str());
		if (content_types->find(extension) != content_types->end()) {
		  if (ss[0] == 'h' && ss[1] == 't') { is_file = 1; } else {
#ifdef ENABLE_COMPRESSION
			compressed = false;
#endif
			is_file = 2; this->add_header(RES_CL, std::to_string(statbuf_.st_size));
			ss = content_types->at(extension); this->add_header(RES_CT, ss);
		  }
		  file_size = statbuf_.st_size; code = 200; if (file_size > BUF_SIZE)is_file = 3;
		  return;
		}
		throw err::not_found(Buffer() << "Content-type of [" << extension << "] is not allowed!");
	  }
	}
	throw err::not_found();
  }
}
