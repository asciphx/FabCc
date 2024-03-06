#include <req-res.hh>
#include <hh/http_error.hh>
#include <app.hh>
namespace fc {
  Req::Req(HTTP m, std::string& u, std::string_view& p, str_map& h, cc::query_string& q, Conn& fib,
    std::unordered_map<std::string_view, std::string_view>& cookie_map, std::unique_ptr<fc::cache_file>& cache, double& max): fiber(fib), length(0),
    method(m), url(u), raw_url(p), headers(h), params(q), cookie_map(cookie_map), cache_file(cache), USE_MAX_MEM_SIZE_MB(max) {}
  // void Req::add_header(std::string key, std::string value) { headers.emplace(std::move(key), std::move(value)); }
  std::string_view Req::header(const std::string_view& k) const { return headers.find(k) != headers.end() ? headers.find(k)->second : RES_empty; }
  std::string_view Req::cookie(const char* k) { if (!cookie_map.size())index_cookies(); return cookie_map[k]; }
  void Req::setTimeoutSec(std::function<void()>&& func, uint32_t seconds) { fiber.timer.add_s(seconds, std::move(func)); }
  void Req::setTimeout(std::function<void()>&& func, uint32_t milliseconds) { fiber.timer.add_ms(milliseconds, std::move(func)); }
  std::string Req::ip_address() const {
    std::string s;
    switch (fiber.in_addr.sa_family) {
    case AF_INET: {
      sockaddr_in* addr_in = (struct sockaddr_in*)&fiber.in_addr;
      s.resize(INET_ADDRSTRLEN);
#if __MINGW32__
      s.append(inet_ntoa(addr_in->sin_addr), INET_ADDRSTRLEN);
#else
      inet_ntop(AF_INET, &(addr_in->sin_addr), const_cast<char*>(s.data()), INET_ADDRSTRLEN);
#endif
      break;
    }
    case AF_INET6: {
      sockaddr_in6* addr_in6 = (struct sockaddr_in6*)&fiber.in_addr;
      s.resize(INET6_ADDRSTRLEN);
#if __MINGW32__
      memcpy(addr_in6->sin6_addr.s6_addr, s.data(), INET6_ADDRSTRLEN);
#else
      inet_ntop(AF_INET6, &(addr_in6->sin6_addr), const_cast<char*>(s.data()), INET6_ADDRSTRLEN);
#endif
      break;
    }
    default: return "unsuported protocol";
    }
    return s;
  }
  void Req::index_cookies() {
    std::string_view cookies = header("Cookie");
    if (!cookies.data()) return;
    const char* line_end = &cookies.back() + 1;
    const char* cur = &cookies.front();
    while (cur < line_end) {
      std::string_view key = std::split(cur, line_end, '=');
      std::string_view value = std::split(cur, line_end, ';');
      while (key[0] == ' ')
        key = std::string_view(key.data() + 1, key.size() - 1);
      cookie_map[key] = value;
    }
  }
  /******************************** ************************************/
  void Res::write(std::string&& b) { Ctx.set_content_type("text/plain;charset=UTF-8", 24); Ctx.respond(std::move(b)); };
  void Res::write_async(std::function<json::Json()>&& f, short i) {
    std::string b = app.get_cache(url); if (!b.empty()) { Ctx.set_content_type("application/json", 16); Ctx.respond(b); return; }
    b = std::move(f().str()); Ctx.set_content_type("application/json", 16); Ctx.respond(b); b.shrink_to_fit(); app.set_cache(url, b, i);
  };
  void Res::write_async_s(std::function<std::string()>&& f, short i) {
    std::string b = app.get_cache(url); if (!b.empty()) { Ctx.respond(b); return; }
    b = std::move(f()); Ctx.respond(b); b.shrink_to_fit(); app.set_cache(url, b, i);
  };
  std::string& Res::compress_str(char* const str, unsigned int len) {
    // Initialize with the default values, level default 8
    char buffer[0x800]; z_stream stream{};
    if (::deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, algorithm::GZIP, 6, Z_DEFAULT_STRATEGY) == Z_OK) {
      stream.avail_in = len; // zlib does not take a const pointer. The data is not altered.
      stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(str));
      code = Z_OK;
      do {
        stream.avail_out = sizeof(buffer);
        stream.next_out = reinterpret_cast<Bytef*>(&buffer[0]);
        code = ::deflate(&stream, Z_FINISH); // Successful and non-fatal error code returned by deflate when used with Z_FINISH flush
        if (code == Z_OK || code == Z_STREAM_END) {
          body.insert(body.end(), &buffer[0], &buffer[sizeof(buffer) - stream.avail_out]);
        }
      } while (code == Z_OK);
      if (code != Z_STREAM_END)
        body.clear();
      ::deflateEnd(&stream);
    }
    return body;
  }
  std::string& Res::decompress_str(char* const str, unsigned int len) {
    Bytef buffer[0x800]; z_stream stream{};
    stream.avail_in = len; // Nasty const_cast but zlib won't alter its contents
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<Bytef const*>(str));
    // Initialize with automatic header detection, for gzip support
    if (::inflateInit2(&stream, MAX_WBITS | 32) == Z_OK) {
      do {
        stream.avail_out = sizeof(buffer);
        stream.next_out = reinterpret_cast<Bytef*>(&buffer[0]);
        auto ret = ::inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_OK || ret == Z_STREAM_END) {
          body.insert(body.end(), &buffer[0], &buffer[sizeof(buffer) - stream.avail_out]);
        } else {
          // Something went wrong with inflate; make sure we return an empty string
          body.clear(); break;
        }
      } while (stream.avail_out == 0); // Free zlib's internal memory
      ::inflateEnd(&stream);
    }
    return body;
  }
}
