#include <req-res.hh>
#include <http_error.hh>
namespace fc {
  // Req::Req():body(0x1ff), params(0x3f), url(0x1f), ip_addr(16) {};
  Req::Req(HTTP m, fc::Buf& u, fc::Buf& p, str_map& h, fc::Buf& b, Conn& fib,
    std::unordered_map<std::string_view, std::string_view>& cookie_map, std::unique_ptr<fc::cache_file>& cacheFile): fiber(fib), length(0),
    method(m), url(u), params(p), headers(h), body(b), cookie_map(cookie_map), cache_file(cacheFile) {}
    // void Req::add_header(fc::Buf key, fc::Buf value) { headers.emplace(std::move(key), std::move(value)); }
  fc::Buf Req::header(const char* k) const { return headers.find(k)->second; }
  std::string_view Req::cookie(const char* k) {
    if (!cookie_map.size())index_cookies(); return cookie_map[k];
  }
  std::string Req::ip_address() const {
    std::string s;
    switch (fiber.in_addr.sa_family) {
    case AF_INET: {
      sockaddr_in* addr_in = (struct sockaddr_in*)&fiber.in_addr;
      s.resize(INET_ADDRSTRLEN);
      //s.append(inet_ntoa(addr_in->sin_addr), INET_ADDRSTRLEN);
      inet_ntop(AF_INET, &(addr_in->sin_addr), const_cast<char*>(s.data()), INET_ADDRSTRLEN);
      break;
    }
    case AF_INET6: {
      sockaddr_in6* addr_in6 = (struct sockaddr_in6*)&fiber.in_addr;
      s.resize(INET6_ADDRSTRLEN);
      inet_ntop(AF_INET6, &(addr_in6->sin6_addr), const_cast<char*>(s.data()), INET6_ADDRSTRLEN);
      break;
    }
    default:
      return "unsuported protocol";
      break;
    }
    return s;
  }
  void Req::index_cookies() {
    fc::Buf cookies = header("Cookie");
    if (!cookies.data()) return;
    const char* line_end = &cookies.back() + 1;
    const char* cur = &cookies.front();
    while (cur < line_end) {
      std::string_view key = split(cur, line_end, '=');
      std::string_view value = split(cur, line_end, ';');
      while (key[0] == ' ')
        key = std::string_view(key.data() + 1, key.size() - 1);
      cookie_map[key] = value;
    }
  }
  std::string_view Req::split(const char*& cur, const char* line_end, char split_char) {
    const char* start = cur;
    while (start < (line_end - 1) && *start == split_char) ++start;
    const char* end = start + 1;
    while (end < (line_end - 1) && *end != split_char) ++end;
    cur = end + 1;
    if (*end == split_char) return std::string_view(start, cur - start - 1);
    else return std::string_view(start, cur - start);
  }
  /******************************** ************************************/
  //Res::Res():zlib_cp_str(0x1ff), body(0xff) {};
  //void Res::set_header(const fc::Buf& key, fc::Buf value) { headers.erase(key); headers.emplace(key, std::move(value)); }
  //void Res::add_header(const fc::Buf& key, fc::Buf value) { headers.emplace(key, std::move(value)); }
  void Res::write(const std::string& b) { std::string_view sv(b.data(), b.size()); Ctx.respond(sv); };
  void Res::write(const std::string_view& b) { Ctx.respond(b); };
  void Res::write(const fc::Buf& b) { std::string_view sv(b.data(), b.size()); Ctx.respond(sv); };
  void Res::write(const char* b) { Ctx.respond(b); };
  fc::Buf& Res::compress_str(char* const str, size_t len) {
  // Initialize with the default values
    if (::deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, algorithm::GZIP, 8, Z_DEFAULT_STRATEGY) == Z_OK) {
      stream.avail_in = (unsigned int)len; // zlib does not take a const pointer. The data is not altered.
      stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(str));
      code = Z_OK;
      do {
        stream.avail_out = sizeof(buffer);
        stream.next_out = reinterpret_cast<Bytef*>(&buffer[0]);
        code = ::deflate(&stream, Z_FINISH); // Successful and non-fatal error code returned by deflate when used with Z_FINISH flush
        if (code == Z_OK || code == Z_STREAM_END) {
          zlib_cp_str.insert(zlib_cp_str.end_, &buffer[0], &buffer[sizeof(buffer) - stream.avail_out]);
        }
      } while (code == Z_OK);
      if (code != Z_STREAM_END)
        zlib_cp_str.reset();
      ::deflateEnd(&stream);
    }
    return zlib_cp_str;
  }
  fc::Buf& Res::decompress_str(char* const str, size_t len) {
    stream.avail_in = (unsigned int)len; // Nasty const_cast but zlib won't alter its contents
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<Bytef const*>(str));
    // Initialize with automatic header detection, for gzip support
    if (::inflateInit2(&stream, MAX_WBITS | 32) == Z_OK) {
      do {
        stream.avail_out = sizeof(buffer);
        stream.next_out = reinterpret_cast<Bytef*>(&buffer[0]);
        auto ret = ::inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_OK || ret == Z_STREAM_END) {
          zlib_cp_str.insert(zlib_cp_str.end_, &buffer[0], &buffer[sizeof(buffer) - stream.avail_out]);
        } else {
          // Something went wrong with inflate; make sure we return an empty string
          zlib_cp_str.reset(); break;
        }
      } while (stream.avail_out == 0); // Free zlib's internal memory
      ::inflateEnd(&stream);
    }
    return zlib_cp_str;
  }
}
