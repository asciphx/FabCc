#include <req-res.hh>
namespace fc {
  void Req::add_header(std::string key, std::string value) {
	headers.emplace(std::move(key), std::move(value));
  }
  const std::string& Req::get_header_value(const std::string& key) const {
	if (headers.count(key)) { return headers.find(key)->second; } return empty;
  }
  /******************************** ************************************/
  void Res::set_header(const std::string& key, std::string value) {
	headers.erase(key); headers.emplace(key, std::move(value));
  }
  void Res::add_header(const std::string& key, std::string value) {
	headers.emplace(key, std::move(value));
  }
  const std::string& Res::get_header_value(const std::string& key) {
	if (headers.count(key)) { return headers.find(key)->second; } return empty;
  }
}
