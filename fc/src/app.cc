#include <app.hh>
#include <directory.hh>
#include <http_error.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
namespace fc {
  char c2m(const char* m) {
	switch (hack8Str(m)) {
	case "DELETE"_l:return static_cast<char>(HTTP::DEL);
	case 4670804:return static_cast<char>(HTTP::GET);
	  //case 1212498244:return static_cast<char>(HTTP::HEAD);
	case 1347375956:return static_cast<char>(HTTP::POST);
	case 5264724:return static_cast<char>(HTTP::PUT);
	  //case "PATCH"_l:return static_cast<char>(HTTP::PATCH);
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
	  //case HTTP::PATCH:return "PATCH";
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
  //VH& App::patch(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::PATCH)); return h; }
  //template <typename Adaptor> //websocket
  //void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
  ///Process the Req and generate a Res for it
  Buffer App::_print_routes() {
	Buffer b(0x1ff); int i = 0; char m;
	map_.for_all_routes([this, &b, &i, &m](std::string r, VH h) {
	  m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
	  b << '(' << ++i << ')' << '[' << m2c((HTTP)m) << ']' <<
		'/' << (r[2] == 0x2f ? r.substr(3) : r.substr(2)) << ',' << (i % 6 == 0 ? '\n' : ' ');
	  }); return b;
  }
  char App::_call(HTTP& m, std::string& r, Req& request, Res& response) const {
	//if (r[r.size() - 1] == '/') r = r.substr(0, r.size() - 1);// skip the last / of the url.
	//std::string g; static_cast<char>(m) < '\12' ? g.push_back(static_cast<char>(m) + 0x30) :
	//  (g.push_back(static_cast<char>(m) % 10 + 0x30), g.push_back(static_cast<char>(m) / 10 + 0x30)); g += r;
	std::string g(1, static_cast<char>(m) + 0x30); g += r;
	fc::drt_node::iterator it = map_.root.find(g, 0); if (it.second != nullptr) {
	  it->second(request, response); return $_(\0);
	}; return $_(\1);
  }
  void App::sub_api(const char* prefix, const App& app) {
	char m; if ((prefix[0] == '/' || prefix[0] == '\\') && prefix[1] == 0)++prefix;
	app.map_.for_all_routes([this, &prefix, &m](std::string r, VH h) {
	  std::string $(prefix); $.push_back('/'); m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
	  $ += r[1] == 0x2f ? r.substr(2) : r.substr(3); this->map_.add($.c_str(), m) = h;
	});
  }
  App App::serve_file(const char* r = STATIC_DIRECTORY) {
	App api;
	try {
	  char real_root[CROSSPLATFORM_MAX_PATH]{ 0 };
	  if (r[0] == 0)r = "."; if (r[0] == '/' || r[0] == '\\')++r;
	  if (!fc::crossplatform_realpath(r, real_root))
		throw err::not_found(fc::Buffer("serve_file error: Directory ", 28) << r << " does not exists.");
	  if (!fc::is_directory(real_root))
		throw err::internal_server_error(fc::Buffer("serve_file error: ", 18) << real_root << " is not a directory.");
	  std::string $(r); if ($.back() != $_(\\) && $.back() != $_(/ )) $.push_back($_(/ )); detail::directory_ = $;
	  api.map_.add("/", static_cast<char>(HTTP::GET)) = [$](Req& req, Res& res) {
		std::string _($); _ += req.url.c_str() + 1; res.write(_); res.write("index.html");
	  };
	  api.map_.add("/*", static_cast<char>(HTTP::GET)) = [$](Req& req, Res& res) {
		std::string _($); _ += req.url.c_str() + 1;// if (_ == $) { res.write("index.html"); return;}
		struct stat statbuf_; stat(_.c_str(), &statbuf_);
		uint64_t length = statbuf_.st_size;
		res.write(_);
	  };
	} catch (const http_error& e) {
	  printf("http_error[%d]: %s", e.i(), e.what().data());
	}
	return api;
  }

} // namespace fc
