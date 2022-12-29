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
  void App::_call(char m, Buf& r, Req& req, Res& res) const {
	//if (r[r.size() - 1] == '/') r = r.substr(0, r.size() - 1);// skip the last / of the url.
	//std::string g; static_cast<char>(m) < '\12' ? g.push_back(static_cast<char>(m) + 0x30) :
	//  (g.push_back(static_cast<char>(m) % 10 + 0x30), g.push_back(static_cast<char>(m) / 10 + 0x30)); g += r;
	std::string g(1, m + 0x30); g.append(r.data_, r.size());// std::cout << m2c(static_cast<HTTP>(m)) << ":" << r << "\n";
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
#endif // __linux__
	} catch (const http_error& e) {
	  printf("http_error[%d]: %s", e.i(), e.what());
	}
	return api;
  }
  void http_serve(App& api, int port, int nthreads, std::string ip) {
	std::function<void(Conn&)> make_http_processor = [&api](Conn& fiber) {
	  try {
		input_buffer rb;
		Ctx ctx(rb, fiber);
		ctx.socket_fd = fiber.socket_fd;
		while (true) {
		  ctx.is_body_read_ = false;
		  ctx.header_lines.clear();
		  ctx.header_lines.reserve(100);
		  // Read until there is a complete header.
		  int header_start = rb.cursor;
		  int header_end = rb.cursor;
		  assert(header_start >= 0);
		  assert(header_end >= 0);
		  assert(ctx.header_lines.size() == 0);
		  ctx.add_header_line(rb.data() + header_end);
		  assert(ctx.header_lines.size() == 1);
		  bool complete_header = false;
		  if (rb.empty())
			if (!rb.read_more(fiber))
			  return;
		  const char* cur = rb.data() + header_end;
		  while (!complete_header) {
			// Look for end of header && save header lines.
			while ((cur - rb.data()) < rb.end - 3) {
			  if (cur[0] == '\r' && cur[1] == '\n') {
				cur += 2; // skip \r\n
				ctx.add_header_line(cur);
				// If we read \r\n twice the header is complete.
				if (cur[0] == '\r' && cur[1] == '\n') {
				  complete_header = true;
				  cur += 2; // skip \r\n
				  header_end = cur - rb.data();
				  break;
				}
			  } else cur++;
			}
			// Read more data from the socket if the headers are not complete.
			if (!complete_header && 0 == rb.read_more(fiber))
			  return;
		  }
		  int failed = llhttp__internal_execute(&ctx.parser_, rb.data(), rb.data() + rb.end);
		  if (failed) { ctx.fiber.shut(_READ); return; }
		  // Header is complete. Process it.
		  // Run the handler.
		  assert(rb.cursor <= rb.end);
		  ctx.body_start = std::string_view(rb.data() + header_end, rb.end - header_end);
		  ctx.prepare_request();
		  Req req = ctx.parser_.to_request<Req>(ctx.fiber);
		  if (!req.url(0)) { req.body = ctx.read_whole_body(); req.url << ctx.url_; };
		  Res res(ctx);
		  try {
			api._call(*((char*)(&req)), req.url, req, res);
		  } catch (const http_error& e) {
			ctx.set_status(e.i());
			ctx.respond(e.what());
		  } catch (const std::exception& e) {
			std::cerr << "INTERNAL SERVER ERROR: " << e.what() << std::endl;
			ctx.set_status(500);
			ctx.respond("Internal server error.");
		  }
		  ctx.respond_if_needed();
		  assert(rb.cursor <= rb.end);
		  // Update the cursor the beginning of the next request.
		  ctx.prepare_next_request();
		  // if read buffer is empty, we can flush the output buffer.
		  if (rb.empty())
			ctx.flush_responses();
		}
	  } catch (const std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return;
	  }
	};
	auto date_thread = std::make_shared<std::thread>([&]() {
	  while (!quit_signal_catched) {
		fc::http_top_header.tick(); std::this_thread::sleep_for(std::chrono::seconds(1));
	  }
	});
	std::cout << "C++<web>[" << static_cast<int>(nthreads) << "] => http://127.0.0.1:" << port << std::endl;
	start_server(ip, port, SOCK_STREAM, nthreads, std::move(make_http_processor));//SOCK_DGRAM
	date_thread->join();
	// if constexpr (has_key<decltype(options)>(s::ssl_key)) {
		//static_assert(has_key<decltype(options)>(s::ssl_certificate),
		//			  "You need to provide both the ssl_certificate option and the ssl_key option.");
		//std::string ssl_key = options.ssl_key;
		//std::string ssl_cert = options.ssl_certificate;
		//std::string ssl_ciphers = get_or(options, s::ssl_ciphers, "");
		//start_tcp_server(ip, port, SOCK_STREAM, nthreads,
		//				 fc::make_http_processor(std::move(handler)), ssl_key, ssl_cert, ssl_ciphers);
	// } else {
	// }
  }
} // namespace fc
