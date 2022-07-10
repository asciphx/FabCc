#include <req-res.hh>
namespace fc {
  void Req::add_header(std::string key, std::string value) {
	headers.emplace(std::move(key), std::move(value));
  }
  /******************************** ************************************/
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
  void Res::end() {
	if (!completed_) {
	  completed_ = true;
	  if (is_head_response) {
		set_header(RES_CL, std::to_string(body.size()));
		body = "";
	  }
	  if (complete_request_handler_) {
		complete_request_handler_();
	  }
	}
  }
}
