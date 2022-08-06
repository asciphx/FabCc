#include <req-res.hh>
#include <http_error.hh>
namespace fc {
  Req::Req():body(0x1ff), params(0x3f), url(0x1f) {};
  Req::Req(HTTP method, fc::Buffer url, fc::Buffer params, str_map headers, fc::Buffer body/*, bool k*/):/* keep_alive(k),*/
	method(method), url(std::move(url)), params(std::move(params)), headers(std::move(headers)), body(std::move(body)) {}
  void Req::add_header(std::string key, std::string value) { headers.emplace(std::move(key), std::move(value)); }
  /******************************** ************************************/
  Res::Res():zlib_cp_str(0x1ff), body(0xff) {};
  void Res::set_header(const std::string& key, std::string value) { headers.erase(key); headers.emplace(key, std::move(value)); }
  void Res::add_header(const std::string& key, std::string value) { headers.emplace(key, std::move(value)); }
  const std::string& Res::get_header(const std::string& key) {
	if (headers.count(key)) { return headers.find(key)->second; } return RES_empty;
  }
  void Res::write(const std::string& body_part) { body << body_part; };
  void Res::write(const fc::Buffer& body_part) { body << body_part; };
  fc::Buffer& Res::compress_str(char* const str, size_t len) {
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
  fc::Buffer& Res::decompress_str(char* const str, size_t len) {
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
