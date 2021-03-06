#include <app.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
namespace fc {
  char sv2c(const char* m) {
	switch (hack8Str(m)) {
	case "DELETE"_l:return static_cast<char>(HTTP::DEL);
	case 4670804:return static_cast<char>(HTTP::GET);
	  //case 1212498244:return static_cast<char>(HTTP::HEAD);
	case 1347375956:return static_cast<char>(HTTP::POST);
	case 5264724:return static_cast<char>(HTTP::PUT);
	case "PATCH"_l:return static_cast<char>(HTTP::PATCH);
	case "OPTIONS"_l:return static_cast<char>(HTTP::OPTIONS);
	  //case "CONNECT"_l:return static_cast<char>(HTTP::CONNECT);
	  //case "TRACE"_l:return static_cast<char>(HTTP::TRACE);
	  //case "PURGE"_l:return static_cast<char>(HTTP::PURGE);
	} return static_cast<char>(HTTP::INVALID);
  }
  const char* m2c(HTTP m) {
	switch (m) {
	case HTTP::DEL:return "DELETE";
	case HTTP::GET:return "GET";
	  //case HTTP::HEAD:return "HEAD";
	case HTTP::POST:return "POST";
	case HTTP::PUT:return "PUT";
	  //case HTTP::CONNECT:return "CONNECT";
	case HTTP::OPTIONS:return "OPTIONS";
	  //case HTTP::TRACE:return "TRACE";
	case HTTP::PATCH:return "PATCH";
	  //case HTTP::PURGE:return "PURGE";
	default:return "invalid";
	}
	return "invalid";
  }
  App::App() {}
  VH& App::operator[](const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::GET)); return h; }
  VH& App::del(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::DEL)); return h; }
  VH& App::get(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::GET)); return h; }
  VH& App::post(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::POST)); return h; }
  VH& App::put(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::PUT)); return h; }
  VH& App::patch(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::PATCH)); return h; }
  //template <typename Adaptor> //websocket
  //void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
  ///Process the Req and generate a Res for it
  Buffer App::_print_routes() {
	Buffer b(0x1ff); int i = 0; char m = 1;
	map_.for_all_routes([this, &b, &i, &m](std::string r, VH h) {
	  m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
	  b << '(' << ++i << ')' << '[' << m2c((HTTP)m) << ']' <<
		'/' << (r[2] == 0x2f ? r.substr(3) : r.substr(2)) << ',' << (i % 6 == 0 ? "<br/>" : " ");
	  }); return b;
  }
  char App::_call(HTTP& m, std::string& r, Req& request, Res& response) const {
	//if (r == last_called_) { last_handler_(request, response); return; }
	//if (r[r.size() - 1] == '/') r = r.substr(0, r.size() - 1);// skip the last / of the url.
	std::string g; static_cast<char>(m) < '\12' ? g.push_back(static_cast<char>(m) + 0x30) :
	  (g.push_back(static_cast<char>(m) % 10 + 0x30), g.push_back(static_cast<char>(m) / 10 + 0x30)); g += r;
	fc::drt_node::iterator it = map_.root.find(g, 0); if (it != DRT_END) {
	  //const_cast<App*>(this)->last_called_ = r; const_cast<App*>(this)->last_handler_ = it->second;
	  it->second(request, response); return '\0';
	}; return '\1';
  }
} // namespace fc
