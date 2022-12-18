#include <app.hh>
#include <list>
#include <utility>
#include <directory.hh>
namespace fc {
  char c2m(const char* m, size_t l) {
	switch (hack_str(m, l)) {
	case 75064423044165:return static_cast<char>(HTTP::DEL);
	case 4670804:return static_cast<char>(HTTP::GET);
	case 1212498244:return static_cast<char>(HTTP::HEAD);
	case 1347375956:return static_cast<char>(HTTP::POST);
	case 5264724:return static_cast<char>(HTTP::PUT);
	  //case 344693424968:return static_cast<char>(HTTP::PATCH);
	case 22324846097550931:return static_cast<char>(HTTP::OPTIONS);
	  //case 18946021178819412:return static_cast<char>(HTTP::CONNECT);
	  //case 362157261637:return static_cast<char>(HTTP::TRACE);
	  //case 345028839237:return static_cast<char>(HTTP::PURGE);
	} return static_cast<char>(HTTP::INVALID);
  }
  const char* m2c(HTTP m) {
	switch (m) {
	case HTTP::DEL:return "DELETE";
	case HTTP::GET:return "GET";
	case HTTP::HEAD:return "HEAD";
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
  Buf App::_print_routes() {
	Buf b(0x1ff); int i = 0; char m;
#ifdef __linux__
	std::list<std::pair<std::string, VH>> aws;
	map_.for_all_routes([this, &aws](std::string r, VH h) { aws.push_front(std::make_pair(r, h)); });
	for (std::pair<std::string, VH>& p : aws) {
	  std::string& r = p.first; VH& h = p.second;
	  m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
	  b << '(' << ++i << ')' << '[' << m2c((HTTP)m) << ']' <<
		'/' << (r[2] == 0x2f ? r.substr(3) : r.substr(2)) << (i % 6 == 0 ? '\n' : ' ') << ',';
	}
#else
	map_.for_all_routes([this, &b, &i, &m](std::string r, VH h) {
	  m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
	  b << '(' << ++i << ')' << '[' << m2c((HTTP)m) << ']' <<
		'/' << (r[2] == 0x2f ? r.substr(3) : r.substr(2)) << (i % 6 == 0 ? '\n' : ' ') << ',';
	});
#endif // _WIN32
	return b.pop_back();
  }
  App& App::file_type(const std::vector<std::string_view>& line) {
	for (std::vector<std::string_view>::const_iterator iter = line.cbegin(); iter != line.cend(); ++iter) {
	  std::string_view sv; sv = content_any_types[*iter];
	  if (sv != "") { content_types.emplace(*iter, sv); } else { content_types.emplace(*iter, RES_oct); }
	} return *this;
  }
  void App::_call(char m, std::string& r, Req& req, Res& res) const {
	//if (r[r.size() - 1] == '/') r = r.substr(0, r.size() - 1);// skip the last / of the url.
	//std::string g; static_cast<char>(m) < '\12' ? g.push_back(static_cast<char>(m) + 0x30) :
	//  (g.push_back(static_cast<char>(m) % 10 + 0x30), g.push_back(static_cast<char>(m) / 10 + 0x30)); g += r;
	std::string g(1, m + 0x30); g.append(r.data(), r.size()); req.url = r;
	//std::cout << m2c(static_cast<HTTP>(m)) << ":" << r << "\n";
	fc::drt_node::iterator it = map_.root.find(g, 0); if (it.second != nullptr) {
	  it->second(req, res);
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
		throw err::not_found(fc::Buf("serve_file error: Directory ", 28) << r << " does not exists.");
	  if (!fc::is_directory(real_root))
		throw err::internal_server_error(fc::Buf("serve_file error: ", 18) << real_root << " is not a directory.");
	  std::string $(r); if ($.back() != '\\' && $.back() != '/') $.push_back('/'); fc::directory_ = $;
#ifndef __linux__
	  api.map_.add("/", static_cast<char>(HTTP::GET)) = [$, this](Req& req, Res& res) {
		std::string _($); _ += req.url.c_str() + 1; _.append("index.html", 10);
		struct stat statbuf_; res.is_file = stat(_.c_str(), &statbuf_);
		if (res.is_file == 0) {
		  res.file_size = statbuf_.st_size;
		  res.set_header("Content-Type", "text/html");
		  std::unordered_map<const std::string, std::shared_ptr<fc::file_sptr>>::iterator p = file_cache_.find(_);
		  if (p != file_cache_.cend() && p->second->modified_time_ == statbuf_.st_mtime) {
			res.__ = p->second;
		  } else {
			file_cache_[_] = res.__ = std::make_shared<file_sptr>(_, (size_t)res.file_size, statbuf_.st_mtime);
		  }
		  res.Ctx.send_file(res.__);
		}
	  };
#endif // !__linux__
	  api.map_.add("/*", static_cast<char>(HTTP::GET)) = [$, this](Req& req, Res& res) {
		std::string _($); _ += req.url.c_str() + 1;// std::cout << _ << "#\n";
		std::string::iterator i = --_.end(); if (*--i == '.')goto _; if (*--i == '.')goto _;
		if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
		if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
		throw err::not_found();
	  _:std::size_t last_dot = $_(i) - $_(_.begin()) + 1;
		struct stat statbuf_; res.is_file = stat(_.c_str(), &statbuf_);
		if (res.is_file == 0 && statbuf_.st_size < BUF_MAXSIZE) {
		  if (last_dot) {
			Buf ss = _.substr(last_dot);
			std::string_view extension(ss.data_, ss.size());// printf("<%d,%s>", res.is_file, _.c_str());
			if (content_types.find(extension) != content_types.end()) {
			  res.file_size = statbuf_.st_size; res.code = 200;
			  ss = content_types.at(extension); res.set_header("Content-Type", ss.b2v());
			  if (extension[0] == 'h' && extension[1] == 't') { res.is_file = 1; } else {
				res.is_file = 2;
				res.set_header("Cache-Control", "max-age=54000,immutable");
			  }
			  std::unordered_map<const std::string, std::shared_ptr<fc::file_sptr>>::iterator p = file_cache_.find(_);
			  if (p != file_cache_.cend() && p->second->modified_time_ == statbuf_.st_mtime) {
				res.__ = p->second;
			  } else {
				file_cache_[_] = res.__ = std::make_shared<file_sptr>(_, (size_t)res.file_size, statbuf_.st_mtime);
			  }
			  if (res.__->ptr_ != nullptr) {
				res.Ctx.send_file(res.__);
			  }
			  //res.path_ = std::move(_);
			  return;
			}
			throw err::not_found(Buf() << "Content-type of [" << extension << "] is not allowed!");
		  }
		}
		throw err::not_found();
	  };
#ifdef __linux__
	  api.map_.add("/", static_cast<char>(HTTP::GET)) = [$](Req& req, Res& res) {
		std::string _($); _ += req.url.c_str() + 1; _.append("index.html", 10); res.Ctx.send_file(_);
	  };
#endif // __linux__
	} catch (const http_error& e) {
	  printf("http_error[%d]: %s", e.i(), e.what());
	}
	return api;
  }
} // namespace fc
