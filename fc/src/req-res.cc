#include <req-res.hh>
namespace fc {
  Req::Req() { body.reserve(0x1ff); params.reserve(0x3f); url.reserve(0x1f); };
  void Req::add_header(std::string key, std::string value) {
	headers.emplace(std::move(key), std::move(value));
  }
  /******************************** ************************************/
  Res::Res() { body.reserve(0xff); path_.reserve(0x3f); };
  void Res::set_header(const std::string& key, std::string value) {
	headers.erase(key); headers.emplace(key, std::move(value));
  }
  void Res::add_header(const std::string& key, std::string value) {
	headers.emplace(key, std::move(value));
  }
  const std::string& Res::get_header(const std::string& key) {
	if (headers.count(key)) { return headers.find(key)->second; } return empty;
  }
  void Res::write(const std::string& body_part) { body += body_part; };
  void Res::set_static_file_info(std::string path) {
	struct stat statbuf_; path_ = detail::directory_; path_ += path.c_str() + 1;
	is_file = stat(path_.c_str(), &statbuf_);
	if (is_file == 0 && statbuf_.st_size < BUF_SIZE) {
	  //std::size_t last_dot = path.find_last_of('.');
	  std::string ss = path.substr(path.find_last_of('.') + 1);
	  std::string_view extension(ss.data(), ss.size());// printf("<%d,%s>", is_file, path_.c_str());
	  if (content_any_types.find(extension) != content_any_types.end()) {
		//if (ss[0] == 'h' && ss[1] == 't') {
		//  is_file = 2; code = 200;
		//  this->add_header(RES_CL, std::to_string(statbuf_.st_size));
		//  ss = content_any_types[extension]; this->add_header(RES_CT, ss);
		//} else
		{
#ifdef ENABLE_COMPRESSION
		  compressed = false;
#endif
		  is_file = 1; code = 200;
		  this->add_header(RES_CL, std::to_string(statbuf_.st_size));
		  ss = content_any_types[extension]; this->add_header(RES_CT, ss);
		}
	  } else {
		code = 404; this->headers.clear(); body = ""; //this->add_header(RES_CT,"text/plain");
	  }
	} else {
	  code = 404;
	}
  }
}
