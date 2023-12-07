#include <ctx.hh>
#include <hh/time.hh>
#ifdef _WIN32
#include <MSWSock.h>
static std::unordered_map<std::string, HANDLE> RES_FILES;
#endif
namespace fc {
  void Ctx::format_top_headers() {
    (output_stream << RES_http_status << status_ << RES_server_tag << fc::server_name_).append("\r\n", 2);
#if ! defined(__MINGW32__)
    output_stream.append("Date: ", 5) << REStop_h.top_header(); output_stream.append(" GMT\r\n", 6);
#endif
  }
  void Ctx::respond(const char* s) {
    format_top_headers(); size_t l = strlen(s); if (content_type[0]) output_stream.append("Content-Type: ", 14)
      .append(content_type).append("\r\n", 2); (output_stream << RES_content_length_tag << l).append("\r\n\r\n", 4).append(s, l);
  }
  void Ctx::respond(std::string&& s) {
    format_top_headers(); if (content_type[0]) output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (output_stream << RES_content_length_tag << s.size()).append("\r\n\r\n", 4) << std::move(s);
  }
  void Ctx::respond(const std::string& s) {
    format_top_headers(); if (content_type[0]) output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (output_stream << RES_content_length_tag << s.size()).append("\r\n\r\n", 4) << s;
  }
  void Ctx::respond(const std::string_view& s) {
    format_top_headers(); if (content_type[0]) output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (output_stream << RES_content_length_tag << s.size()).append("\r\n\r\n", 4) << const_cast<std::string_view&>(s);
  }
  void Ctx::set_header(std::string_view& k, std::string_view& v) { output_stream.append(k).append(": ", 2).append(v).append("\r\n", 2); }
  void Ctx::set_header(std::string_view&& k, std::string_view&& v) { output_stream.append(k).append(": ", 2).append(v).append("\r\n", 2); }
  void Ctx::set_cookie(std::string_view k, std::string_view v) {
    (output_stream.append("Set-Cookie: ", 12).append(k) << '=').append(v).append("\r\n", 2);
  }//(\"\d+[ A-Za-z-]+\\r\\n\")
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
  // Send a file_sptr.(can support larger than 4GB)
  void Ctx::send_file_sptr(std::shared_ptr<fc::file_sptr>& __, _Fsize_t p, long long size) {
    output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (output_stream << RES_content_length_tag << __->size_).append("\r\n\r\n", 4); output_stream.flush();
    __->read_chunk(p, static_cast<int>(size > INT32_MAX ? INT32_MAX : size), [this](const char* s, int l, std::function<void()> d) {
      if (l > 0) { if (this->fiber.writen(s, l) && d != nullptr) d(); } });
#ifdef _WIN32
    if (errno == EINPROGRESS || errno == EINVAL) fiber.shut(_WRITE);
#else
    if (errno == EAGAIN) fiber.shut(_WRITE);
#endif
  }
  // send file with pos
  void Ctx::send_file_p(std::string& path, _Fsize_t pos, long long sizer) {
#ifndef _WIN32 // Linux / Macos version with sendfile(can support larger than 4GB)
    // Open the file in non blocking mode.
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) { content_type = RES_NIL; throw err::not_found(path << " => not found."); }
    long file_size = lseek(fd, (size_t)0, SEEK_END);
    // Writing the http headers.
    output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (output_stream << RES_content_length_tag << file_size).append("\r\n\r\n", 4);
    output_stream.flush();
    off_t offset = pos; lseek(fd, pos, SEEK_SET);
    while (offset < sizer) {
#if __APPLE__ // sendfile on macos is slightly different...
      off_t nwritten = 0;
      int ret = ::sendfile(fd, fiber.socket_fd, offset, &nwritten, nullptr, 0);
      offset += nwritten;
      if (ret == 0 && nwritten == 0) break; // end of file.
#else
      int ret = ::sendfile(fiber.socket_fd, fd, &offset, sizer - offset);
#endif
      if (ret != -1) {
        if (offset < sizer) {
          continue; // this->fiber.rpg->_ = this->fiber.rpg->_.yield();
        }
      } else if (errno == EPIPE) {
        break;
      } else {
        if (errno != EAGAIN) {
          close(fd); //std::cerr << "Internal error: sendfile failed: " << strerror(errno) << std::endl;
          throw err::not_found("sendfile failed.");
        }
        this->fiber.rpg->_ = this->fiber.rpg->_.yield();
      }
    }
    if (errno == EAGAIN) fiber.shut(_WRITE);
    close(fd);
#else // Windows impl with basic sned_file method.(not support larger than 4GB)
    DWORD g_BytesTransferred = 0;
    OVERLAPPED overlap;
    HANDLE fd; LARGE_INTEGER fileSize;
#ifdef __MINGW32__
    auto af = RES_FILES.find(path);
    if (af == RES_FILES.end()) {
#else
    if ((fd = RES_FILES.find(path)->second) == nullptr) {
#endif
      int path_len = ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
      WCHAR* pwsz = new WCHAR[path_len]; ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, pwsz, path_len);
      fd = ::CreateFileW(pwsz, FILE_GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); delete[] pwsz; pwsz = nullptr;
      if (fd == INVALID_HANDLE_VALUE) {
        content_type = RES_NIL; throw err::not_found(path << " => not found.");
      }
      RES_FILES[path] = fd;
    }
#ifdef __MINGW32__
    else fd = af->second;
#endif
    memset(&overlap, 0, sizeof(overlap)); overlap.Offset = pos; overlap.OffsetHigh = (DWORD)((pos >> 0x20) & 0xFFFFFFFFL);
    GetFileSizeEx(fd, &fileSize); if (fileSize.QuadPart > UINT32_MAX) {
      CloseHandle(fd); content_type = RES_NIL; throw err::not_extended("Can't larger than 4GB!");
    }
    content_length_ = static_cast<long long>(fileSize.QuadPart);
    // Writing the http headers.
    output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (output_stream << RES_content_length_tag << content_length_).append("\r\n\r\n", 4);
    output_stream.flush();
    while (overlap.Offset < sizer) {
      bool rc = ReadFile(fd, output_stream.buffer_, static_cast<DWORD>(output_stream.cap_), &g_BytesTransferred, &overlap);
      if (rc) {
        overlap.Offset += g_BytesTransferred; if(!this->fiber.writen(output_stream.buffer_, g_BytesTransferred)) break;
      } else {
        if (GetLastError() == ERROR_IO_PENDING) {
          if (errno == EPIPE) break;
          if (errno == EINPROGRESS || errno == EINVAL) {
            WaitForSingleObject(fd, INFINITE);//INFINITE
            rc = GetOverlappedResult(fd, &overlap, &g_BytesTransferred, FALSE);
            if (rc) {
              overlap.Offset += g_BytesTransferred;
              if(this->fiber.writen(output_stream.buffer_, overlap.Offset > sizer ? overlap.Offset - sizer : g_BytesTransferred)) continue;
            }
          }
        }
        throw err::bad_request(":Err!");
      }
    }
    if (errno == EINPROGRESS || errno == EINVAL) fiber.shut(_WRITE);
    content_length_ = 0;
#endif
  }
  // Send a file.
  void Ctx::send_file(std::string & path, bool is_download) {
#ifndef _WIN32 // Linux / Macos version with sendfile
    // Open the file in non blocking mode.
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) { content_type = RES_NIL; throw err::not_found(path << " => not found."); }
    long file_size = lseek(fd, (size_t)0, SEEK_END);
    // Writing the http headers.
    output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    if (is_download) (output_stream << RES_content_length_tag << file_size).append("\r\n\r\n", 4);
    else output_stream.append("Transfer-Encoding: chunked\r\n\r\n", 30);
    output_stream.flush();
    off_t offset = 0; lseek(fd, 0, 0);
    while (offset < file_size) {
#if __APPLE__ // sendfile on macos is slightly different...
      off_t nwritten = 0;
      int ret = ::sendfile(fd, fiber.socket_fd, offset, &nwritten, nullptr, 0);
      offset += nwritten;
      if (ret == 0 && nwritten == 0) break; // end of file.
#else
      int ret = ::sendfile(fiber.socket_fd, fd, &offset, file_size - offset);
#endif
      if (ret != -1) {
        if (offset < file_size) {
          continue; // this->fiber.rpg->_ = this->fiber.rpg->_.yield();
        }
      } else if (errno == EAGAIN) {
        this->fiber.rpg->_ = this->fiber.rpg->_.yield();
      } else {
        close(fd);
        //std::cerr << "Internal error: sendfile failed: " << strerror(errno) << std::endl;
        throw err::not_found("sendfile failed.");
      }
    }
    close(fd);
#else // Windows impl with basic read write.
    DWORD g_BytesTransferred = 0;
    OVERLAPPED overlap;
    HANDLE fd; LARGE_INTEGER fileSize;
#ifdef __MINGW32__
    auto af = RES_FILES.find(path);
    if (af == RES_FILES.end()) {
#else
    if ((fd = RES_FILES.find(path)->second) == nullptr) {
#endif
      int path_len = ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
      WCHAR* pwsz = new WCHAR[path_len]; ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, pwsz, path_len);
      fd = ::CreateFileW(pwsz, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL); delete[] pwsz; pwsz = nullptr;
      if (fd == INVALID_HANDLE_VALUE) {
        content_type = RES_NIL; throw err::not_found(path << " => not found.");
      }
      RES_FILES[path] = fd;
    }
#ifdef __MINGW32__
    else fd = af->second;
#endif
    GetFileSizeEx(fd, &fileSize); if (fileSize.QuadPart > UINT32_MAX) {
      CloseHandle(fd); content_type = RES_NIL; throw err::not_extended("Can't larger than 4GB!");
    }
    content_length_ = static_cast<long long>(fileSize.QuadPart);
    // Writing the http headers.
    output_stream.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    if (is_download) (output_stream << RES_content_length_tag << content_length_).append("\r\n\r\n", 4);
    else output_stream.append("Transfer-Encoding: chunked\r\n\r\n", 30);
    output_stream.flush();
    //::TransmitFile(socket_fd, fd, 0, 16777216, &overlap, NULL, TF_DISCONNECT | TF_REUSE_SOCKET);//CloseHandle(fd);
    memset(&overlap, 0, sizeof(overlap)); overlap.OffsetHigh = (DWORD)((content_length_ >> 0x20) & 0xFFFFFFFFL);
    while (overlap.Offset < content_length_) {
      bool rc = ReadFile(fd, output_stream.buffer_, static_cast<DWORD>(output_stream.cap_), &g_BytesTransferred, &overlap);
      if (rc) {
        overlap.Offset += g_BytesTransferred; this->fiber.write(output_stream.buffer_, g_BytesTransferred);
      } else {
        if (GetLastError() == ERROR_IO_PENDING) {
          if (errno == EINPROGRESS || errno == EINVAL) {
            WaitForSingleObject(fd, INFINITE);//INFINITE
            rc = GetOverlappedResult(fd, &overlap, &g_BytesTransferred, FALSE);
            if (rc) {
              if (this->fiber.write(output_stream.buffer_, g_BytesTransferred)) { overlap.Offset += g_BytesTransferred; continue; }
            }
          }
        }
        throw err::bad_request(":Err!");
      }
    }
    content_length_ = 0;
#endif
  }
  void Ctx::prepare_next_request() {
    status_ = std::string_view("200 OK\r\n", 8); content_type = RES_NIL;
#ifdef _WIN32
    fiber.epoll_mod(EPOLLIN | EPOLLRDHUP);
#endif // _WIN32
  }
}