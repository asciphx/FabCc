#include "ctx.hh"
#include "hh/time.hh"
#ifdef _WIN32
#include <MSWSock.h>
static std::unordered_map<std::string, HANDLE, fc::str_hash, fc::str_key_eq> RES_FILES;
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
  _CTX_TASK(void) Ctx::send_file_sptr(std::shared_ptr<fc::file_sptr>& __, _Fsize_t p, long long size) {
    ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (ot << RES_content_length_tag << __->size_).append("\r\n\r\n", 4); co_await ot.flush();
    __->read_chunk(p, static_cast<int>(size > INT32_MAX ? INT32_MAX : size),
      [this](const char* s, int l, std::function<void()> d)_ctx{
      if (l > 0) { if ((co_await this->fiber.writen(s, l)) && d != nullptr) d(); }co_return; });
#ifdef _WIN32
    if (errno == EINPROGRESS || errno == EINVAL) fiber.shut(_WRITE);
#else
    if (errno == EAGAIN) fiber.shut(_WRITE);
#endif
    time(&fiber.rpg->hrt); co_return;
  }
  // send file with pos
  _CTX_TASK(void) Ctx::send_file_p(std::string& path, _Fsize_t pos, long long sizer) {
#ifndef _WIN32 // Linux / Macos version with sendfile(can support larger than 4GB)
    // Open the file in non blocking mode.
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) { content_type = RES_NIL; throw err::not_found(path << " => not found."); }
    //// Plan to add a flow control strategy, especially for non-member video viewing
    long file_size = sizer - pos;
    //// File block transfer controls the size of each block, which can be used for flow control
    // if (file_size > 16777216) file_size = 16777216, sizer = pos + 16777216;
    // Writing the http headers.
    ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (ot << RES_content_length_tag << file_size).append("\r\n\r\n", 4);
    co_await ot.flush();
    off_t offset = pos; lseek(fd, pos, SEEK_SET);
    do {
#if __APPLE__ // sendfile on macos is slightly different...
      off_t nwritten = 0;
      int ret = ::sendfile(fd, fiber.socket_fd, offset, &nwritten, nullptr, 0);
      offset += nwritten;
      if (ret == 0 && nwritten == 0) break; // end of file.
#else
      int ret = ::sendfile(fiber.socket_fd, fd, &offset, sizer - offset);
#endif
      if (ret == -1) {
        if (errno == EPIPE) {
          break;
        } else if (errno != EAGAIN) {
          close(fd); //std::cerr << "Internal error: sendfile failed: " << strerror(errno) << std::endl;
          throw err::not_found("sendfile failed.");
        }
#if __cplusplus < _cpp20_date
        this->fiber.rpg->_.operator()();
#else
        co_await std::suspend_always{};
#endif
      }
    } while (offset < sizer);
    close(fd);
    if (errno == EAGAIN) fiber.shut(_WRITE);
#else // Windows impl with basic sned_file method.(not support larger than 4GB)
    HANDLE fd; int path_len = ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
    WCHAR* pwsz = new WCHAR[path_len]; ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, pwsz, path_len);
    fd = ::CreateFileW(pwsz, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
      OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, NULL); delete[] pwsz; pwsz = nullptr;
    if (fd == INVALID_HANDLE_VALUE) { content_type = RES_NIL; throw err::not_found(path << " => not found."); }
    // #ifdef __MINGW32__
    //     auto af = RES_FILES.find(path);
    //     if (af == RES_FILES.end()) {
    // #else
    //     if ((fd = RES_FILES.find(path)->second) == nullptr) {
    // #endif
    //       int path_len = ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
    //       WCHAR* pwsz = new WCHAR[path_len]; ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, pwsz, path_len);
    //       fd = ::CreateFileW(pwsz, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); delete[] pwsz; pwsz = nullptr;
    //       if (fd == INVALID_HANDLE_VALUE) {
    //         content_type = RES_NIL; throw err::not_found(path << " => not found.");
    //       }
    //       RES_FILES[path] = fd;
    //     }
    // #ifdef __MINGW32__
    //     else fd = af->second;
    // #endif
    // LARGE_INTEGER fileSize; GetFileSizeEx(fd, &fileSize); if (fileSize.QuadPart > UINT32_MAX) {
    //   CloseHandle(fd); content_type = RES_NIL; throw err::not_extended("Can't larger than 4GB!");
    // }
    //// Plan to add a flow control strategy, especially for non-member video viewing
    content_length_ = sizer - pos;//static_cast<long long>(fileSize.QuadPart);
    //// File block transfer controls the size of each block, which can be used for flow control
    // if (content_length_ > 16777216) content_length_ = 16777216, sizer = pos + 16777216;
    // Writing the http headers.
    ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    (ot << RES_content_length_tag << content_length_).append("\r\n\r\n", 4);
    OVERLAPPED ov = { 0 }; ov.Offset = pos; DWORD g_Bytes = 0; size_t z = ot.size(); char p[0x2000];
    memcpy(p, ot.buffer_, z); bool rc = ReadFile(fd, p + z, static_cast<DWORD>(sizeof(p) - z), &g_Bytes, &ov);
    if (rc) {
      ov.Offset += g_Bytes; if (!co_await this->fiber.writen(p, static_cast<int>(z) + g_Bytes)) goto _;
    } else {
      if (GetLastError() == ERROR_IO_PENDING) {
        if (errno == EPIPE) { goto _; }
        WaitForSingleObject(fd, 1000);//INFINITE
        if (rc = GetOverlappedResult(fd, &ov, &g_Bytes, TRUE)) {
          ov.Offset += g_Bytes; if (!co_await this->fiber.writen(p, static_cast<int>(z) + g_Bytes)) goto _;
        }
      }
    }
    do {
      bool rc = ReadFile(fd, p, static_cast<DWORD>(sizeof(p)), &g_Bytes, &ov);
      if (rc) {
        ov.Offset += g_Bytes; if (!co_await this->fiber.writen(p, g_Bytes)) goto _;
      } else {
        if (GetLastError() == ERROR_IO_PENDING) {
          if (errno == EPIPE) { goto _; }
          if (errno == EINPROGRESS || errno == EINVAL) {
            WaitForSingleObject(fd, INFINITE);//INFINITE
            if (rc = GetOverlappedResult(fd, &ov, &g_Bytes, FALSE)) {
              ov.Offset += g_Bytes;
              if (co_await this->fiber.writen(p, g_Bytes)) continue;
            }
          }
        }
        goto _;
      }
    } while (ov.Offset < sizer);
    if (errno == EINPROGRESS || errno == EINVAL) fiber.shut(_WRITE); _: CloseHandle(fd);
    content_length_ = 0; ::setsockopt(fiber.socket_fd, SOL_SOCKET, SO_LINGER, (const char*)&RESling, sizeof(linger));
#endif
    co_return;
  }
  // Send a file.
  _CTX_TASK(void) Ctx::send_file(std::string& path, bool is_download) {
    //// Plan to add a flow control strategy, especially for downloading
#ifndef _WIN32 // Linux / Macos version with sendfile
    // Open the file in non blocking mode.
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) { content_type = RES_NIL; throw err::not_found(path << " => not found."); }
    long file_size = lseek(fd, (size_t)0, SEEK_END);
    // Writing the http headers.
    ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    if (is_download) (ot << RES_content_length_tag << file_size).append("\r\n\r\n", 4);
    else ot.append("Transfer-Encoding: chunked\r\n\r\n", 30);
    co_await ot.flush();
    off_t offset = 0; lseek(fd, 0, 0);
    do {
#if __APPLE__ // sendfile on macos is slightly different...
      off_t nwritten = 0;
      int ret = ::sendfile(fd, fiber.socket_fd, offset, &nwritten, nullptr, 0);
      offset += nwritten;
      if (ret == 0 && nwritten == 0) break; // end of file.
#else
      int ret = ::sendfile(fiber.socket_fd, fd, &offset, file_size - offset);
#endif
      if (ret != -1 && offset < file_size) {
        continue;
      } else if (errno == EAGAIN) {
#if __cplusplus < _cpp20_date
        this->fiber.rpg->_.operator()();
#else
        co_await std::suspend_always{};
#endif
      } else {
        close(fd);
        //std::cerr << "Internal error: sendfile failed: " << strerror(errno) << std::endl;
        throw err::not_found("sendfile failed.");
      }
    } while (offset < file_size);
    close(fd);
#else // Windows impl with basic read write.
    HANDLE fd;
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
      if (fd == INVALID_HANDLE_VALUE) { content_type = RES_NIL; throw err::not_found(path << " => not found."); }
      RES_FILES[path] = fd;
    }
#ifdef __MINGW32__
    else fd = af->second;
#endif
    LARGE_INTEGER fileSize; GetFileSizeEx(fd, &fileSize); if (fileSize.QuadPart > UINT32_MAX) {
      CloseHandle(fd); content_type = RES_NIL; throw err::not_extended("Can't larger than 4GB!");
    }
    content_length_ = static_cast<long long>(fileSize.QuadPart);
    // Writing the http headers.
    ot.append("Content-Type: ", 14).append(content_type).append("\r\n", 2);
    if (is_download) (ot << RES_content_length_tag << content_length_).append("\r\n\r\n", 4);
    else ot.append("Transfer-Encoding: chunked\r\n\r\n", 30);
    co_await ot.flush();
    //::TransmitFile(socket_fd, fd, 0, 16777216, &ov, NULL, TF_DISCONNECT | TF_REUSE_SOCKET);//CloseHandle(fd);
    DWORD g_Bytes = 0; OVERLAPPED ov; memset(&ov, 0, sizeof(ov)); ov.OffsetHigh = (DWORD)((content_length_ >> 0x20) & 0xFFFFFFFFL);
    do {
      bool rc = ReadFile(fd, ot.buffer_, static_cast<DWORD>(ot.cap_), &g_Bytes, &ov);
      if (rc) {
        ov.Offset += g_Bytes; co_await this->fiber.write(ot.buffer_, g_Bytes);
      } else {
        if (GetLastError() == ERROR_IO_PENDING) {
          if (errno == EINPROGRESS || errno == EINVAL) {
            WaitForSingleObject(fd, 1000);//INFINITE
            rc = GetOverlappedResult(fd, &ov, &g_Bytes, FALSE);
            if (rc) {
              if (co_await this->fiber.write(ot.buffer_, g_Bytes)) { ov.Offset += g_Bytes; continue; }
            }
          }
        }
        break;
      }
    } while (ov.Offset < content_length_);
    content_length_ = 0;
#endif
    co_return;
  }
  void Ctx::prepare_next_request() {
    status_ = std::string_view("200 OK\r\n", 8); content_type = RES_NIL;
#ifdef _WIN32
    fiber.epoll_mod(EPOLLIN | EPOLLRDHUP);
#endif // _WIN32
  }
}