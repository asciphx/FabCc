#ifndef REQRES_HH
#define REQRES_HH
#include <functional>
#include <h/common.h>

namespace fc {
  class Conn; class Tcp;
  class Req {
	friend class fc::Tcp;
	HTTP method;
	std::string raw_url;
  public:
	std::string url;
	str_map headers;
	std::string body;
	std::string ip_addr;
	void add_header(std::string key, std::string value);
  };// request

  class Res {
	friend class fc::Conn;
	friend class fc::Tcp;
	str_map headers;
	std::string path_;
	int is_file{ 0 };
	bool completed_{};
	std::function<void()> complete_request_handler_;
  public:
	bool is_head_response = false;
	uint16_t code{ 200 };// Check whether the response has a static file defined.
	std::string body;
	void set_header(const std::string& key, std::string value);
	void add_header(const std::string& key, std::string value);
	const std::string& get_header(const std::string& key);
	void write(const std::string& body_part);
	void end();
  };// response
}

#endif // REQRES_HH
