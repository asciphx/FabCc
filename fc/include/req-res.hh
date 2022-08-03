#ifndef REQRES_HH
#define REQRES_HH
#include <functional>
#include <h/common.h>
#include <timer.hh>
#define BUF_SIZE 0x28000
#define BUF_HTML_MAXSIZE 0x3e801
#define BUF_MAXSIZE 256000000
namespace fc {
  class Conn; class Tcp; struct App;
  class Req {
	friend class fc::Tcp;
	HTTP method;
  public:
	Req();
	Req(HTTP method, std::string url, std::string params, str_map headers, std::string body);
	std::string url;
	std::string params;
	std::string body;
	uint64_t uuid;
	str_map headers;
	std::string ip_addr;
	void add_header(std::string key, std::string value);
  };// request

  class Res {
	friend class fc::Conn;
	friend class fc::Tcp;
	friend struct fc::App;
	str_map headers;
	std::string path_;
	fc::Timer timer_;
	int is_file{ 0 };
	long file_size = 0;
	std::function<void(int64_t o, int64_t k,std::function<void(const char* c, size_t s, std::function<void()> d)> sink)> provider;
	Res();
  public:
	uint16_t code{ 200 };// Check whether the response has a static file defined.
	std::string body;
	void set_header(const std::string& key, std::string value);
	void add_header(const std::string& key, std::string value);
	const std::string& get_header(const std::string& key);
	void write(const std::string& body_part);
  };// response
}

#endif // REQRES_HH
