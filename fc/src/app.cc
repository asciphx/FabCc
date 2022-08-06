#include <app.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/api.hh
#ifdef _WIN32
#define $_(_) _._Ptr
#else
#define $_(_) _.base()
#endif // _WIN32
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
  void App::_call(HTTP& m, fc::Buffer& r, Req& request, Res& response) const {
	//if (r[r.size() - 1] == '/') r = r.substr(0, r.size() - 1);// skip the last / of the url.
	//std::string g; static_cast<char>(m) < '\12' ? g.push_back(static_cast<char>(m) + 0x30) :
	//  (g.push_back(static_cast<char>(m) % 10 + 0x30), g.push_back(static_cast<char>(m) / 10 + 0x30)); g += r;
	std::string g(1, static_cast<char>(m) + 0x30); g += r.b2v();
	fc::drt_node::iterator it = map_.root.find(g, 0); if (it.second != nullptr) {
	  it->second(request, response);
	} else throw err::not_found();
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
	  std::string $(r); if ($.back() != '\\' && $.back() != '/') $.push_back('/'); detail::directory_ = $;
	  api.map_.add("/", static_cast<char>(HTTP::GET)) = [$](Req& req, Res& res) {
		std::string _($); _ += req.url.c_str() + 1; _ += "index.html";
		struct stat statbuf_; res.is_file = stat(_.c_str(), &statbuf_);
		if (res.is_file == 0 && statbuf_.st_size < BUF_HTML_MAXSIZE) {
		  std::string::iterator i = --_.end(); if (*--i == '.')goto _; if (*--i == '.')goto _;
		  if (*--i == '.')goto _; if (*--i == '.')goto _; throw err::not_found();
		_:std::size_t last_dot = $_(i) - $_(_.begin()) + 1;
		  if (last_dot) {
			std::string ss = _.substr(last_dot);
			if (ss[0] == 'h' && ss[1] == 't') {
			  res.is_file = 1; res.file_size = statbuf_.st_size; res.code = 200; res.path_ = std::move(_);
			}
		  }
		}
	  };
	  api.map_.add("/*", static_cast<char>(HTTP::GET)) = [$, this](Req& req, Res& res) {
		std::string _($); _ += req.url.c_str() + 1;// if (_ == $) { res.write("index.html"); return;}
		struct stat statbuf_; res.is_file = stat(_.c_str(), &statbuf_);
		if (res.is_file == 0 && statbuf_.st_size < BUF_MAXSIZE) {
		  std::string::iterator i = --_.end(); if (*--i == '.')goto _; if (*--i == '.')goto _;
		  if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
		  if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
		  throw err::not_found();
		_:std::size_t last_dot = $_(i) - $_(_.begin()) + 1;
		  if (last_dot) {
			std::string ss = _.substr(last_dot);
			std::string_view extension(ss.data(), ss.size());// printf("<%d,%s>", res.is_file, _.c_str());
			if (content_types->find(extension) != content_types->end()) {
			  res.file_size = statbuf_.st_size; res.code = 200;
			  if (ss[0] == 'h' && ss[1] == 't') { res.is_file = 1; } else {
#ifdef ENABLE_COMPRESSION
				compressed = false;
#endif
				res.is_file = 2; res.add_header(RES_CL, std::to_string(statbuf_.st_size));
				ss = content_types->at(extension); res.add_header(RES_CT, ss);
				std::shared_ptr<file_sptr> __;
				std::unordered_map<const std::string, std::shared_ptr<fc::file_sptr>>::iterator p = file_cache_.find(_);
				if (p != file_cache_.cend() && p->second->modified_time_ == statbuf_.st_mtime) {
				  __ = p->second;
				} else {
				  file_cache_[_] = __ = std::make_shared<file_sptr>(_, (size_t)res.file_size, statbuf_.st_mtime);
				}
				if (__ && __->ptr_ != nullptr) {
				  res.provider = [__](int64_t o, int64_t k,
				  std::function<void(const char* c, size_t l, std::function<void()> f)> sink) {
					//size_t l = min(BUF_MAXSIZE, (size_t)(k - o));
					int r = __->read_chunk(o, k - o, sink); if (r < 0 && r != UV_EAGAIN) { sink(nullptr, r, nullptr); return; }
					//_:o += l; if (o < k) { goto _; }if(__.use_count()>2)__.~shared_ptr();// printf("[%ld]",__.use_count());
				  };
				}
			  }
			  //printf("<%ld,%s>", res.file_size, _.c_str());
			  res.path_ = std::move(_);
			  return;
			}
			throw err::not_found(Buffer() << "Content-type of [" << extension << "] is not allowed!");
		  }
		}
		throw err::not_found();
	  };
	} catch (const http_error& e) {
	  printf("http_error[%d]: %s", e.i(), e.what().data());
	}
	return api;
  }

} // namespace fc
