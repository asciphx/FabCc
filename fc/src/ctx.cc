#include <ctx.hh>
namespace fc {
  void Ctx::format_top_headers(output_buffer& output_stream) {
    if (status_code_ == 200) output_stream << http_top_header.top_header_200();
    else output_stream << "HTTP/1.1 " << status_ << http_top_header.top_header();
  }
  void Ctx::prepare_request() {
    response_headers.clear(); content_length_ = chunked_ = 0;
    for (int i = 1; i < header_lines.size() - 1; ++i) {
      const char* line_end = header_lines[i + 1]; // last line is just an empty line.
      const char* cur = header_lines[i]; if (*cur != 'C' && *cur != 'c') continue;
      std::string_view key = split(cur, line_end, ':');
      auto get_value = [&] {
        std::string_view value = split(cur, line_end, '\r');
        while (value[0] == ' ') value = std::string_view(value.data() + 1, value.size() - 1);
        return value;
      };
      if (key == "Content-Length") {
        content_length_ = std::lexical_cast<unsigned int>(get_value());
      } else if (key == "Content-Type") {
        content_type_ = get_value(); chunked_ = (content_type_ == "chunked");
      }
    }
  }
  void Ctx::respond(const std::string_view& s) {
    response_written_ = true;
    format_top_headers(output_stream);
    headers_stream.flush();                                             // flushes to output_stream.
    output_stream << "Content-Length: " << s.size() << "\r\n\r\n" << const_cast<std::string_view&>(s); // Add body
    //std::cout << output_stream.to_string_view() << "$\n";
  }
  void Ctx::respond_if_needed() {
    if (!response_written_) {
      response_written_ = true;
      format_top_headers(output_stream);
      output_stream << headers_stream.to_string_view();
      output_stream << "Content-Length: 0\r\n\r\n"; // Add body
    }
  }
  void Ctx::set_header(std::string_view k, std::string_view v) {
    headers_stream << k << ": " << v << "\r\n";
  }
  void Ctx::set_cookie(std::string_view k, std::string_view v) {
    headers_stream << "Set-Cookie: " << k << '=' << v << "\r\n";
  }
  void Ctx::set_status(int status) {
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
    default: status_ = "200 OK";
    }
  }
  // Send a file.
  void Ctx::send_file(std::string& path) {
#ifndef _WIN32 // Linux / Macos version with sendfile
  // Open the file in non blocking mode.
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) throw err::not_found(Buf("File:", 5) << path << " not found.");
    size_t file_size = lseek(fd, (size_t)0, SEEK_END);
    // Writing the http headers.
    response_written_ = true;
    format_top_headers(output_stream);
    headers_stream.flush(); // flushes headers to output_stream.
    output_stream << "Content-Length: " << file_size << "\r\n\r\n";
    output_stream.flush();
    off_t offset = 0; lseek(fd, (size_t)0, 0);
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
        this->fiber.sink = this->fiber.sink.yield();
      } else {
        close(fd); std::cerr << "Internal error: sendfile failed: " << strerror(errno) << std::endl;
        throw err::not_found("Internal error: sendfile failed.");
      }
    }
    close(fd);
#else // Windows impl with basic read write.
  // Open file.
    FILE* fd; if ((fd = fopen(path.c_str(), "rb")) == NULL) // C4996
      throw err::not_found(Buf("File:", 5) << path << " not found.");
    fseek(fd, 0L, SEEK_END); content_length_ = static_cast<unsigned int>(ftell(fd)); rewind(fd);
    // Writing the http headers.
    response_written_ = true; format_top_headers(output_stream);
    headers_stream.flush(); // flushes to output_stream.
    output_stream << "Content-Length: " << content_length_ << "\r\n\r\n"; // Add body
    output_stream.flush();
    // Read the file and write it to the socket.
    char buffer[32768];
    while (_fread_nolock(buffer, sizeof(buffer), 1, fd) != 0) {
      content_length_ -= sizeof(buffer); this->fiber.write(buffer, sizeof(buffer));
    }
    _fread_nolock(buffer, content_length_, 1, fd);
    this->fiber.write(buffer, content_length_); fclose(fd); content_length_ = 0;
#endif
  }
  // split a string, starting from cur && ending with split_char.
  // Advance cur to the end of the split.
  std::string_view Ctx::split(const char*& cur, const char* line_end, char split_char) {
    const char* start = cur;
    while (start < (line_end - 1) && *start == split_char) ++start;
    const char* end = start + 1;
    while (end < (line_end - 1) && *end != split_char) ++end;
    cur = end + 1;
    if (*end == split_char) return std::string_view(start, cur - start - 1);
    else return std::string_view(start, cur - start);
  }
  std::string_view Ctx::read_whole_body() {
    is_body_read_ = true; if (!chunked_ && !content_length_) {
      body_end_ = body_start.data(); return std::string_view(); // No body.
    }
    if (content_length_) {
      body_ = rb.read_n(fiber, body_start.data(), content_length_);
      body_end_ = body_.data() + content_length_;
      //std::cout << '[' << body <<" : " << content_length_ << ']';
    } else if (chunked_) { // Chunked decoding.
      char* out = (char*)body_start.data(); const char* cur = body_start.data();
      int chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16); ++cur; // skip \n 
      while (chunked_size > 0) { // Read chunk.
        std::string_view chunk = rb.read_n(fiber, cur, chunked_size); cur += chunked_size + 2; // skip \r\n.
        // Copy the body into a contiguous string.
        if (out + chunk.size() > chunk.data()) // use memmove if overlap.
          std::memmove(out, chunk.data(), chunk.size()); else std::memcpy(out, chunk.data(), chunk.size());
        out += chunk.size(); // Read next chunk size.
        chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16); ++cur; // skip \n
      } cur += 2; // skip the terminaison chunk.
      body_end_ = cur; body_ = std::string_view(body_start.data(), out - body_start.data());
    } return body_;
  }
  void Ctx::prepare_next_request() {
    if (!is_body_read_) read_whole_body(); rb.free(header_lines[0], body_end_);
    headers_stream.reset(); status_ = "200 OK"; content_type_ = std::string_view();
    response_headers.clear(); response_written_ = false;
  }
}