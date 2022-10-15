#ifndef CTX_H
#define CTX_H
#include <cstring>
#include <functional>
#include <iostream>
#include <hpp/string_view.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#endif
#if __linux__ // the sendfile header does not exists on macos.
#include <sys/sendfile.h>
#endif
#include <unordered_map>
#include <lexical_cast.hh>
#include <http_error.hh>
#include <tcp.hh>
#include <h/any_types.h>
#include <http_top_header_builder.hh>
#include <buf.hh>
#include <input_buffer.hh>
#include <output_buffer.hh>
namespace fc {
  static char* date_buf = nullptr; static int date_buf_size = 0;
  using ::fc::content_types; // static std::unordered_map<std::string_view, std::string_view>
  // content_types
  static thread_local std::unordered_map<std::string, std::pair<std::string_view, std::string_view>>
	static_files;
#ifndef _WIN32
  http_top_header_builder http_top_header [[gnu::weak]];
#else
  __declspec(selectany) http_top_header_builder http_top_header;
#endif
  struct Ctx {
	Ctx(input_buffer& _rb, Conn& _fiber): rb(_rb), fiber(_fiber), method_(2) {
	  get_parameters_map.reserve(10); response_headers.reserve(20);
	  output_stream = output_buffer(50 * 1024, [&](const char* d, int s) { fiber.write(d, s); });
	  headers_stream = output_buffer(1000, [&](const char* d, int s) { output_stream << std::string_view(d, s); });
	}
	Ctx& operator=(const Ctx&) = delete;
	Ctx(const Ctx&) = delete;
	std::string_view header(const char* key) {
	  if (!header_map.size())
		index_headers();
	  return header_map[key];
	}
	std::string_view cookie(const char* key) {
	  if (!cookie_map.size())
		index_cookies();
	  return cookie_map[key];
	}
	std::string_view get_parameter(const char* key) {
	  if (!url_.size())
		parse_first_line();
	  return get_parameters_map[key];
	}
	fc::Buf& url() {
	  if (!url_.size())
		parse_first_line();
	  return url_;
	}
	fc::Buf& method() {
	  if (!method_.size())
		parse_first_line();
	  return method_;
	}
	std::string_view http_version() {
	  if (!url_.size())
		parse_first_line();
	  return http_version_;
	}

	inline void format_top_headers(output_buffer& output_stream) {
	  if (status_code_ == 200)
		output_stream << http_top_header.top_header_200();
	  else
		output_stream << "HTTP/1.1 " << status_ << http_top_header.top_header();
	}

	void prepare_request() {
	  // parse_first_line();
	  response_headers.clear();
	  content_length_ = 0;
	  chunked_ = 0;
	  for (int i = 1; i < header_lines.size() - 1; i++) {
		const char* line_end = header_lines[i + 1]; // last line is just an empty line.
		const char* cur = header_lines[i];
		if (*cur != 'C' && *cur != 'c')
		  continue;
		std::string_view key = split(cur, line_end, ':');
		auto get_value = [&] {
		  std::string_view value = split(cur, line_end, '\r');
		  while (value[0] == ' ')
			value = std::string_view(value.data() + 1, value.size() - 1);
		  return value;
		};

		if (key == "Content-Length")
		  content_length_ = atoi(get_value().data());
		else if (key == "Content-Type") {
		  content_type_ = get_value();
		  chunked_ = (content_type_ == "chunked");
		}
	  }
	}
	void respond(const fc::Buf& s) {
	  response_written_ = true;
	  format_top_headers(output_stream);
	  headers_stream.flush();                                             // flushes to output_stream.
	  output_stream << "Content-Length: " << s.size() << "\r\n\r\n" << s.b2v(); // Add body
	  //std::cout << output_stream.to_string_view() << "$\n";
	}

	void respond_if_needed() {
	  if (!response_written_) {
		response_written_ = true;
		format_top_headers(output_stream);
		output_stream << headers_stream.to_string_view();
		output_stream << "Content-Length: 0\r\n\r\n"; // Add body
	  }
	}

	void set_header(std::string_view k, std::string_view v) {
	  headers_stream << k << ": " << v << "\r\n";
	}

	void set_cookie(std::string_view k, std::string_view v) {
	  headers_stream << "Set-Cookie: " << k << '=' << v << "\r\n";
	}

	void set_status(int status) {
	  status_code_ = status;
	  switch (status) {
	  case 200:status_ = "200 OK"; break;
	  case 201:status_ = "201 Created"; break;
	  case 202:status_ = "202 Accepted"; break;
	  case 203:status_ = "203 Non-Authoritative Information"; break;
	  case 204:status_ = "204 No Content"; break;
	  case 206:status_ = "206 Partial Content"; break;

	  case 301:status_ = "301 Moved Permanently"; break;
	  case 302:status_ = "302 Found"; break;
	  case 303:status_ = "303 See Other"; break;
	  case 304:status_ = "304 Not Modified"; break;
	  case 307:status_ = "307 Temporary redirect"; break;

	  case 400:status_ = "400 Bad Request"; break;
	  case 401:status_ = "401 Unauthorized"; break;
	  case 402:status_ = "402 Payment Required"; break;
	  case 403:status_ = "403 Forbidden"; break;
	  case 404:status_ = "404 Not Found"; break;
	  case 405:status_ = "405 HTTP verb used to access this page is not allowed"; break;
	  case 406:status_ = "406 Browser does not accept the MIME type of the requested page"; break;
	  case 409:status_ = "409 Conflict"; break;

	  case 500:status_ = "500 Internal Server Error"; break;
	  case 501:status_ = "501 Not Implemented"; break;
	  case 502:status_ = "502 Bad Gateway"; break;
	  case 503:status_ = "503 Service Unavailable"; break;
	  case 510:status_ = "510 Not Extended"; break;
	  default: status_ = "200 OK"; break;
	  }
	}

	// Send a file.
	void send_file(const char* path) {
#ifndef _WIN32 // Linux / Macos version with sendfile
	  // Open the file in non blocking mode.
	  int fd = open(path, O_RDONLY);
	  if (fd == -1) throw err::not_found(Buf("File:", 5) << path << " not found.");
	  size_t file_size = lseek(fd, (size_t)0, SEEK_END);
	  // Set content type header.
	  size_t ext_pos = std::string_view(path).rfind('.');
	  std::string_view content_type("");
	  if (ext_pos != std::string::npos) {
		auto type_itr = content_types.find(std::string_view(path).substr(ext_pos + 1).data());
		if (type_itr != content_types.end()) {
		  content_type = type_itr->second;
		  set_header("Content-Type", content_type);
		}
	  }
	  // Writing the http headers.
	  response_written_ = true;
	  format_top_headers(output_stream);
	  headers_stream.flush(); // flushes headers to output_stream.
	  output_stream << "Content-Length: " << file_size << "\r\n\r\n";
	  output_stream.flush();
	  off_t offset = 0;
	  lseek(fd, (size_t)0, 0);
	  while (offset < file_size) {
#if __APPLE__ // sendfile on macos is slightly different...
		off_t nwritten = 0;
		int ret = ::sendfile(fd, socket_fd, offset, &nwritten, nullptr, 0);
		offset += nwritten;
		if (ret == 0 && nwritten == 0) break; // end of file.
#else
		int ret = ::sendfile(socket_fd, fd, &offset, file_size - offset);
#endif
		if (ret != -1) {
		  if (offset < file_size) {
			continue; // this->fiber.sink.yield();
		  }
		} else if (errno == EAGAIN) {
		  this->fiber.sink.yield();
		} else {
		  close(fd);
		  std::cerr << "Internal error: sendfile failed: " << strerror(errno) << std::endl;
		  throw err::not_found("Internal error: sendfile failed.");
		}
	  }

	  close(fd);

#else // Windows impl with basic read write.
	  size_t ext_pos = std::string_view(path).rfind('.');
	  std::string_view content_type;
	  if (ext_pos != std::string::npos) {
		auto type_itr = content_types.find(std::string_view(path).substr(ext_pos + 1).data());
		if (type_itr != content_types.end()) {
		  content_type = type_itr->second; set_header("Content-Type", content_type);
		  set_header("Cache-Control", "max-age=54000,immutable");
		}
	  }
	  // Open file.
	  FILE* fd; if ((fd = fopen(path, "r")) == NULL) // C4996
		throw err::not_found(Buf("File:", 5) << path << " not found.");
	  fseek(fd, 0L, SEEK_END);
	  // Get file size.
	  long file_size = ftell(fd);
	  // Writing the http headers.
	  response_written_ = true;
	  format_top_headers(output_stream);
	  headers_stream.flush(); // flushes to output_stream.
	  output_stream << "Content-Length: " << file_size << "\r\n\r\n"; // Add body
	  output_stream.flush();
	  rewind(fd);
	  // Read the file and write it to the socket.
	  size_t nread = 1;
	  size_t offset = 0;
	  while (nread != 0) {
		char buffer[4096];
		nread = _fread_nolock(buffer, sizeof(buffer), 1, fd);
		offset += sizeof(buffer);
		this->fiber.write(buffer, sizeof(buffer));
	  }
	  char buffer[4096];
	  nread = _fread_nolock(buffer, file_size - offset, 1, fd);
	  this->fiber.write(buffer, file_size - offset);
	  if (!feof(fd))
		throw err::not_found("Internal error: Could not reach the end of file.");
	  fclose(fd);
#endif
	  }
	void add_header_line(const char* l) { header_lines.push_back(l); }
	const char* last_header_line() { return header_lines.back(); }
	// split a string, starting from cur && ending with split_char.
	// Advance cur to the end of the split.
	std::string_view split(const char*& cur, const char* line_end, char split_char) {
	  const char* start = cur;
	  while (start < (line_end - 1) && *start == split_char)
		start++;
#if 0
	  const char* end = (const char*)memchr(start + 1, split_char, line_end - start - 2);
	  if (!end) end = line_end - 1;
#else
	  const char* end = start + 1;
	  while (end < (line_end - 1) && *end != split_char)
		end++;
#endif
	  cur = end + 1;
	  if (*end == split_char)
		return std::string_view(start, cur - start - 1);
	  else
		return std::string_view(start, cur - start);
	}
	void index_headers() {
	  for (int i = 1; i < header_lines.size() - 1; i++) {
		const char* line_end = header_lines[i + 1]; // last line is just an empty line.
		const char* cur = header_lines[i];
		std::string_view key = split(cur, line_end, ':');
		std::string_view value = split(cur, line_end, '\r');
		while (value[0] == ' ')
		  value = std::string_view(value.data() + 1, value.size() - 1);
		header_map[key] = value;
		// std::cout << key << " -> " << value << std::endl;
	  }
	}
	void index_cookies() {
	  std::string_view cookies = header("Cookie");
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
	template <typename C> void url_decode_parameters(std::string_view content, C kv_callback) {
	  const char* c = content.data();
	  const char* end = c + content.size();
	  if (c < end) {
		while (c < end) {
		  std::string_view key = split(c, end, '=');
		  std::string_view value = split(c, end, '&');
		  kv_callback(key, value);
		  // printf("kv: '%.*s' -> '%.*s'\n", key.size(), key.data(), value.size(), value.data());
		}
	  }
	}
	void parse_first_line() {
	  const char* c = header_lines[0];
	  const char* end = header_lines[1];
	  method_ = split(c, end, ' ');
	  url_ = split(c, end, ' ');
	  http_version_ = split(c, end, '\r');
	  // url get parameters.
	  c = url_.data();
	  end = c + url_.size();
	  url_ = split(c, end, '?');
	  get_parameters_string_ = std::string_view(c, end - c);
	}
	std::string_view get_parameters_string() {
	  if (!get_parameters_string_.data())
		parse_first_line();
	  return get_parameters_string_;
	}
	template <typename F> void parse_get_parameters(F processor) {
	  url_decode_parameters(get_parameters_string(), processor);
	}
	template <typename F> void read_body(F callback) {
	  is_body_read_ = true;
	  if (!chunked_ && !content_length_)
		body_end_ = body_start.data();
	  else if (content_length_) {
		std::string_view res;
		int n_body_read = 0;

		// First return part of the body already in memory.
		int l = std::min(int(body_start.size()), content_length_);
		callback(std::string_view(body_start.data(), l));
		n_body_read += l;

		while (content_length_ > n_body_read) {
		  std::string_view part = rb.read_more_str(fiber);
		  int l = part.size();
		  int bl = std::min(l, content_length_ - n_body_read);
		  part = std::string_view(part.data(), bl);
		  callback(part);
		  rb.free(part);
		  body_end_ = part.data();
		  n_body_read += part.size();
		}

	  } else if (chunked_) {
		// Chunked decoding.
		assert(0);
		// const char* cur = body_start.data();
		// int chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
		// cur++; // skip \n
		// while (chunked_size > 0) {
		//   // Read chunk.
		//   std::string_view chunk = rb.read_n(read, cur, chunked_size);
		//   callback(chunk);
		//   rb.free(chunk);
		//   cur += chunked_size + 2; // skip \r\n.

		//   // Read next chunk size.
		//   chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
		//   cur++; // skip \n
		// }
		// cur += 2; // skip the terminaison chunk.
		// body_end_ = cur;
		// body_ = std::string_view(body_start.data(), cur - body_start.data());
	  }
	}

	std::string_view read_whole_body() {
	  if (!chunked_ && !content_length_) {
		is_body_read_ = true;
		body_end_ = body_start.data();
		return std::string_view(); // No body.
	  }
	  if (content_length_) {
		body_ = rb.read_n(fiber, body_start.data(), content_length_);
		body_end_ = body_.data() + content_length_;
	  } else if (chunked_) {
		// Chunked decoding.
		char* out = (char*)body_start.data();
		const char* cur = body_start.data();
		int chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16);
		cur++; // skip \n
		while (chunked_size > 0) {
		  // Read chunk.
		  std::string_view chunk = rb.read_n(fiber, cur, chunked_size);
		  cur += chunked_size + 2; // skip \r\n.
		  // Copy the body into a contiguous string.
		  if (out + chunk.size() > chunk.data()) // use memmove if overlap.
			std::memmove(out, chunk.data(), chunk.size());
		  else
			std::memcpy(out, chunk.data(), chunk.size());

		  out += chunk.size();

		  // Read next chunk size.
		  chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16);
		  cur++; // skip \n
		}
		cur += 2; // skip the terminaison chunk.
		body_end_ = cur;
		body_ = std::string_view(body_start.data(), out - body_start.data());
	  }
	  is_body_read_ = true;
	  return body_;
	}
	void read_multipart_formdata() {}

	template <typename F> void post_iterate(F kv_callback) {
	  if (is_body_read_) // already in memory.
		url_decode_parameters(body_, kv_callback);
	  else // stream the body.
	  {
		std::string_view current_key;
		read_body([](std::string_view part) {
		  // read key if needed.
		  // if (!current_key.size())
		  // call kv callback if some value is available.
		  });
	  }
	}
	// Read post parameters in the body.
	std::unordered_map<std::string_view, std::string_view> post_parameters() {
	  if (content_type_ == "application/x-www-form-urlencoded") {
		if (!is_body_read_)
		  read_whole_body();
		url_decode_parameters(body_, [&](auto key, auto value) { post_parameters_map[key] = value; });
		return post_parameters_map;
	  } else {
		// fixme: return bad request here.
	  }
	  return post_parameters_map;
	}

	void prepare_next_request() {
	  if (!is_body_read_)
		read_whole_body();
	  rb.free(header_lines[0], body_end_);
	  headers_stream.reset();
	  status_ = "200 OK";
	  method_ = std::string_view();
	  url_ = std::string_view();
	  http_version_ = std::string_view();
	  content_type_ = std::string_view();
	  header_map.clear();
	  cookie_map.clear();
	  response_headers.clear();
	  get_parameters_map.clear();
	  post_parameters_map.clear();
	  get_parameters_string_ = std::string_view();
	  response_written_ = false;
	}
	void flush_responses() { output_stream.flush(); }
	fc::socket_type socket_fd;
	input_buffer& rb;
	int status_code_ = 200;
	const char* status_ = "200 OK";
	fc::Buf method_;
	fc::Buf url_;
	std::string_view http_version_;
	std::string_view content_type_;
	bool chunked_;
	int content_length_;
	std::unordered_map<std::string_view, std::string_view> header_map;
	std::unordered_map<std::string_view, std::string_view> cookie_map;
	std::vector<std::pair<std::string_view, std::string_view>> response_headers;
	std::unordered_map<std::string_view, std::string_view> get_parameters_map;
	std::unordered_map<std::string_view, std::string_view> post_parameters_map;
	std::string_view get_parameters_string_;
	// std::vector<std::string> strings_saver;
	bool is_body_read_ = false;
	std::string body_local_buffer_;
	std::string_view body_;
	std::string_view body_start;
	const char* body_end_ = nullptr;
	std::vector<const char*> header_lines;
	fc::Conn& fiber;
	output_buffer headers_stream;
	bool response_written_ = false;
	output_buffer output_stream;
  };

} // namespace fc

#endif