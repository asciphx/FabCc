#include "ctx.hh"
#include "hh/time.hh"
#ifdef _WIN32
#include <MSWSock.h>
#include <handleapi.h>
#endif
namespace fc {
  void Ctx::respond(size_t s, str_map& map) {
    for (auto& kv : map) ot.append(kv.first).append(RES_seperator).append(kv.second).append(RES_crlf);
    if (content_type[0]) ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (ot << RES_content_length_tag << s).append("\r\n\r\n", 4);
  }
  void Ctx::add_header(const std::string_view& k, const std::string_view& v) { ot.append(k).append(": ", 2).append(v).append("\r\n", 2); }
  void Ctx::add_header(const std::string_view& k, const char* v) { ot.append(k).append(": ", 2) << v; ot.append("\r\n", 2); }
  void Ctx::add_header(const std::string_view& k, std::string&& v) { ot.append(k).append(": ", 2).append(std::move(v)).append("\r\n", 2); }
  void Ctx::set_cookie(std::string_view k, std::string_view v) {
    (ot.append("Set-Cookie: ", 12).append(k) << '=').append(v).append("\r\n", 2);
  }//(\"\d+[ A-Za-z-]+\\r\\n\")
  int Ctx::get_http_version() { return this->http_minor; }
  void Ctx::set_status(int status) {
    switch (status) {
    case 200:status_ = std::string_view("200 OK\r\n", 8); break;
    case 201:status_ = std::string_view("201 Created\r\n", 13); break;
    case 202:status_ = std::string_view("202 Accepted\r\n", 14); break;
    case 203:status_ = std::string_view("203 Non-Authoritative Information\r\n", 35); break;
    case 204:status_ = std::string_view("204 No Content\r\n", 16); break;
    case 206:status_ = std::string_view("206 Partial Content\r\n", 21); break;

    case 301:status_ = std::string_view("301 Moved Permanently\r\n", 23); break;
    case 302:status_ = std::string_view("302 Found\r\n", 11); break;
    case 303:status_ = std::string_view("303 See Other\r\n", 15); break;
    case 304:status_ = std::string_view("304 Not Modified\r\n", 18); break;
    case 307:status_ = std::string_view("307 Temporary redirect\r\n", 24); break;

    case 400:status_ = std::string_view("400 Bad Request\r\n", 17); break;
    case 401:status_ = std::string_view("401 Unauthorized\r\n", 18); break;
    case 402:status_ = std::string_view("402 Payment Required\r\n", 22); break;
    case 403:status_ = std::string_view("403 Forbidden\r\n", 15); break;
    case 404:status_ = std::string_view("404 Not Found\r\n", 15); break;
    case 405:status_ = std::string_view("405 HTTP verb used to access this page is not allowed\r\n", 55); break;
    case 406:status_ = std::string_view("406 Browser does not accept the MIME type of the requested page\r\n", 65); break;
    case 409:status_ = std::string_view("409 Conflict\r\n", 14); break;
    case 413:status_ = std::string_view("413 Payload Too Large\r\n", 23); break;

    case 500:status_ = std::string_view("500 Internal Server Error\r\n", 27); break;
    case 501:status_ = std::string_view("501 Not Implemented\r\n", 21); break;
    case 502:status_ = std::string_view("502 Bad Gateway\r\n", 17); break;
    case 503:status_ = std::string_view("503 Service Unavailable\r\n", 25); break;
    default: status_ = std::string_view("510 Not Extended\r\n", 18);
    }
  }
  _CTX_TASK(void) Ctx::send_file(const std::shared_ptr<fc::file_sptr>& __, _Fsize_t p, long long size) {
    ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2); content_length_ = size - p;
    // if (content_length_ > 16777216) content_length_ = 16777216, size = p + 16777216;
    (ot << RES_content_length_tag << content_length_).append("\r\n\r\n", 4); co_await ot.flush();
    co_await __->read_chunk([this, size, p](_Fhandle fd)_ctx{
#ifdef _WIN32
      OVERLAPPED ov { 0 }; ov.Offset = p;
      do {
        nwritten = 0;
        if (ReadFile(fd, ot.buffer_, static_cast<DWORD>(ot.cap_), &nwritten, &ov)) {
          if (co_await this->fiber.write(ot.buffer_, nwritten)) { ov.Offset += nwritten; continue; }
        } else {
          if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(fd, 1000);//INFINITE
            if (GetOverlappedResult(fd, &ov, &nwritten, FALSE)) {
              if (co_await this->fiber.write(ot.buffer_, nwritten)) { ov.Offset += nwritten; continue; }
            }
          }
          throw err::not_found("failed.");
        }
        break;
      } while (ov.Offset < size);
#else
      off_t ov = p; lseek(fd, p, SEEK_SET);
      do {
#if _OPENSSL
        if ((ret = read(fd, ot.buffer_, static_cast<int>(ot.cap_))) < 1) break;
        if (co_await this->fiber.write(ot.buffer_, ret)) { ov += ret; continue; }
        if (errno == EPIPE) break; throw err::not_found("failed.");
#else
#if __APPLE__ // sendfile on macos is slightly different...
        nwritten = 0;
        ret = ::sendfile(fd, fiber.socket_fd, ov, &nwritten, nullptr, 0);
        ov += nwritten;
        if (ret == 0 && nwritten == 0) break; // end of file.
#else
        ret = ::sendfile(fiber.socket_fd, fd, &ov, size - ov);
#endif
        if (ret == -1) {
          if (errno == EPIPE) break; throw err::not_found("failed.");
        } else if (errno == EAGAIN) {
#if __cplusplus < _cpp20_date
          this->fiber.rpg->_.operator()();
#else
          co_await std::suspend_always{};
#endif
        }
#endif
      } while (ov < size);
#endif
    co_return; });
#ifdef _WIN32
    if (errno == EINPROGRESS || errno == EINVAL)fiber.shut(_WRITE), ::setsockopt(fiber.socket_fd, SOL_SOCKET, SO_LINGER, (const char*)&RESling, sizeof(linger));
#else
    if (errno == EAGAIN)fiber.shut(_WRITE);
#endif
    if(__.use_count() == 1) const_cast<std::shared_ptr<fc::file_sptr>&>(__) = std::make_shared<file_sptr>();
    content_length_ = 0; time(&fiber.rpg->hrt); co_return;
  }
  // Send a file.
  _CTX_TASK(void) Ctx::send_file(const std::shared_ptr<fc::file_sptr>& __, bool is_download) {
    content_length_ = static_cast<long long>(__->size_);
    // Writing the http headers.
    ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    if (is_download) (ot << RES_content_length_tag << content_length_).append("\r\n\r\n", 4);
    else ot.append("Transfer-Encoding: chunked\r\n\r\n", 30);
    // Open the file in non blocking mode.
    co_await __->read_chunk([this](_Fhandle fd)_ctx{
#ifndef _WIN32 // Linux / Macos version with sendfile
      // if (fd == -1) { content_type = RES_NIL; throw err::not_found(); }
      co_await ot.flush(); off_t ov = 0;
      do {
#if _OPENSSL
        if ((ret = read(fd, ot.buffer_, static_cast<int>(ot.cap_))) < 1) break;
        if (co_await this->fiber.write(ot.buffer_, ret)) { ov += ret; continue; }
        if (errno == EPIPE) break; throw err::not_found("failed.");
#else
#if __APPLE__ // sendfile on macos is slightly different...
        nwritten = 0;
        ret = ::sendfile(fd, fiber.socket_fd, ov, &nwritten, nullptr, 0);
        ov += nwritten;
        if (ret == 0 && nwritten == 0) break; // end of file.
#else
        ret = ::sendfile(fiber.socket_fd, fd, &ov, content_length_ - ov);
#endif
        if (ret == -1) {
          if (errno == EPIPE) break; throw err::not_found("failed.");
        } else if (errno == EAGAIN) {
#if __cplusplus < _cpp20_date
          this->fiber.rpg->_.operator()();
#else
          co_await std::suspend_always{};
#endif
        }
#endif
      } while (ov < content_length_);
#else // Windows impl with basic read write.
      // if (fd == nullptr) { content_type = RES_NIL; throw err::not_found(); }
      co_await ot.flush(); OVERLAPPED ov { 0 }; ov.OffsetHigh = (DWORD)((content_length_ >> 0x20) & 0xFFFFFFFFL);
      do {
        nwritten = 0;
        if (ReadFile(fd, ot.buffer_, static_cast<DWORD>(ot.cap_), &nwritten, &ov)) {
          if (co_await this->fiber.write(ot.buffer_, nwritten)) { ov.Offset += nwritten; continue; }
        } else {
          if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(fd, 1000);//INFINITE
            if (GetOverlappedResult(fd, &ov, &nwritten, FALSE)) {
              if (co_await this->fiber.write(ot.buffer_, nwritten)) { ov.Offset += nwritten; continue; }
            }
          }
          throw err::not_found("failed.");
        }
        break;
      } while (ov.Offset < content_length_);
#endif
    co_return; });
    content_length_ = 0; if(__.use_count() == 1) const_cast<std::shared_ptr<fc::file_sptr>&>(__) = std::make_shared<file_sptr>(); co_return;
  }
  void Ctx::prepare_next_request() {
    status_ = std::string_view("200 OK\r\n", 8); content_type = RES_NIL;
#ifdef _WIN32
    fiber.epoll_mod(EPOLLIN | EPOLLRDHUP);
#endif // _WIN32
  }
}