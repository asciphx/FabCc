#ifndef CTX_H
#define CTX_H
#include <cstring>
#include <functional>
#include <iostream>
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
#include "hh/lexical_cast.hh"
#include "hh/http_error.hh"
#include "hpp/output_buffer.hpp"
#include "hh/tcp.hh"
#include "h/any_types.h"
#include "hpp/string_view.hpp"
#include "hh/str_map.hh"
#include "hh/cache_file.hh"
#include "hh/query_string.hh"
#include "file_sptr.hh"
namespace fc {
  struct Ctx {
    output_buffer ot;//output_stream
    std::unordered_map<std::string_view, std::string_view, sv_hash, sv_key_eq> cookie_map;
    std::string_view content_type, status_;
    std::unique_ptr<fc::cache_file> cache_file;
    Conn& fiber;
    _Fsize_t content_length_;
#ifdef _WIN32
    DWORD nwritten;
#elif __APPLE__
    off_t nwritten;
#else
    int ret;
#endif
    int http_minor{ 0 };
    Ctx(Conn& _fiber, char* rb, size_t l): fiber(_fiber), content_type("", 1), content_length_(0),
      ot(rb, static_cast<int>(l), &_fiber), status_("200 OK\r\n", 8) {}
    Ctx& operator=(const Ctx&) = delete;
    Ctx(const Ctx&) = delete;
    _FORCE_INLINE void format_top_headers() {
      (ot << RES_http_status << status_ << RES_server_tag << fc::server_name_).append("\r\n", 2);
#if ! defined(__MINGW32__)
      ot.append("Date: ", 5) << REStop_h.top_header(); ot.append(" GMT\r\n", 6);
#endif
    }
    //1 =  HTTP/1.1, 0 = HTTP/1.0
    int get_http_version();
    void respond(size_t s, str_map& map);
    _FORCE_INLINE void set_content_type(const std::string_view& sv) { if (!content_type[0])content_type = sv; };
    _FORCE_INLINE void set_content_type(const char* v, size_t&& l) { if (!content_type[0])content_type = std::string_view(v, l); };
    void add_header(const std::string_view& k, const std::string_view& v);
    void add_header(const std::string_view& k, const char* v);
    void add_header(const std::string_view& k, std::string&& v);
    void set_cookie(std::string_view k, std::string_view v);
    void set_status(int status);
    // Send a file_sptr with pos
    _CTX_TASK(void) send_file(const std::shared_ptr<fc::file_sptr>& __, _Fsize_t pos, long long size);
    // Send a file.
    _CTX_TASK(void) send_file(const std::shared_ptr<fc::file_sptr>& __, bool pos = true);
    // split a string, starting from cur && ending with split_char.
    // Advance cur to the end of the split.
    void prepare_next_request();
  };
} // namespace fc
#endif