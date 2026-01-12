#ifndef REQRES_HH
#define REQRES_HH
#include <functional>
#include <string>
#include <iostream>
#include "tp/ctx.hh"
#include "ctx.hh"
#include "hh/conn.hh"
#include "h/common.h"
#include "hpp/string_view.hpp"
#include "hh/directory.hh"
#include "file_sptr.hh"
#include "json.hh"
#if defined(_MSC_VER)
#include <io.h>
#else
#include <fcntl.h>
#endif
namespace fc {
  struct App;
  class Res;
  class Req {
    int is_file{ 0 }; friend Res;
    void index_cookies();
  public:
    HTTP method;
    std::string_view cookie(const char* k);
    std::string ip_address() const;
    Req(HTTP method, std::string& url, std::string_view& params, fc::sv_map& headers, cc::query_string& q, Conn& fib, double& max,
      fc::sv_map& cookie_map, std::unique_ptr<fc::cache_file>& cache_file);
    std::string& url;
    std::string_view& raw_url;
    cc::query_string& params;
    double& USE_MAX_MEM_SIZE_MB;
    _Fsize_t length;
    fc::sv_map& headers;
    Conn& fiber;
    std::unique_ptr<fc::cache_file>& cache_file;
    fc::sv_map& cookie_map;
    std::string_view body;
    void setTimeoutSec(std::function<void()>&& func, uint32_t seconds = 1);
    void setTimeout(std::function<void()>&& func, uint32_t milliseconds = 100);
  };// request
  enum algorithm { // 15 is the default value for deflate
    DEFLATE = 15, // windowBits can also be greater than 15 for optional gzip encoding.
    // Add 16 to windowBits to write a simple gzip header and trailer around the compressed data instead of a zlib wrapper.
    GZIP = 15 | 16,
  };
  class Res {
    fc::Ctx& ctx;
    std::string body, mask_url;
    friend class fc::Conn;
    friend struct fc::App;
  public:
    fc::App& app;
    str_map headers; //`res.headers["..."] = "...";` to set headers
    _FORCE_INLINE void set_cookie(std::string_view k, std::string_view v) { ctx.set_cookie(k, v); }
    _FORCE_INLINE Res(fc::Ctx& ctx, App* ap): ctx(ctx), app(*ap) {}
    //Generally used to read configuration files, or slow io operations, return Json
    void write_async(std::function<json::Json()>&& f, short i = CACHE_HTML_TIME_SECOND);
    //Generally used to read configuration files, or slow io operations, return string
    void write_async_s(std::function<std::string()>&& f, short i = CACHE_HTML_TIME_SECOND);
    _FORCE_INLINE void write(const json::Json& j) { ctx.content_type = std::string_view("application/json", 16); body.append(j.str()); };
    _FORCE_INLINE void write(const std::string& b) { ctx.content_type = std::string_view("text/plain;charset=UTF-8", 24); body.append(b.c_str(), b.length()); };
    _FORCE_INLINE void write(const std::string_view& b) { ctx.content_type = std::string_view("text/plain;charset=UTF-8", 24); body.append(b.data(), b.size()); };
    //After write
    _FORCE_INLINE void set_content_type(const char* v) { ctx.content_type = std::string_view(v, strlen(v)); };
    //After write
    _FORCE_INLINE void set_content_type(const char* v, size_t l) { ctx.content_type = std::string_view(v, std::move(l)); };
    //After set_content_type
    _FORCE_INLINE void write(const char* b) { body.append(b); };
    inline void set_status(int s) { ctx.set_status(s); }
    std::string& compress_str(char* const str, unsigned int len);
    std::string& decompress_str(char* const str, unsigned int len);
    // Location can either be a route or a full URL.
    void redirect(const std::string& location, bool always = false);
  };// response
}

#endif // REQRES_HH
