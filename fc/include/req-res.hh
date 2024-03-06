#ifndef REQRES_HH
#define REQRES_HH
#include <functional>
#include <string>
#include "tp/zlib.h"
#include "tp/ctx.hh"
#include <ctx.hh>
#include <hh/conn.hh>
#include <h/common.h>
#include <hpp/string_view.hpp>
#include <hh/str_map.hh>
#include <hh/directory.hh>
#include <file_sptr.hh>
#include <json.hh>
#include <iostream>
#if defined(_MSC_VER)
#include <io.h>
#else
#include <fcntl.h>
#endif
namespace fc {
  static std::unordered_map<std::string, std::string> RES_CACHE_MENU = {};
  static std::unordered_map<std::string, int64_t> RES_CACHE_TIME = {};
  struct App;
  class Res;
  class Req {
    HTTP method; friend Res;
    void index_cookies();
  public:
    std::string_view header(const std::string_view& k) const;
    std::string_view cookie(const char* k);
    std::string ip_address() const;
    Req(HTTP method, std::string& url, std::string_view& params, str_map& headers, cc::query_string& q, Conn& fib,
      std::unordered_map<std::string_view, std::string_view>& cookie_map, std::unique_ptr<fc::cache_file>& cache_file, double& max);
    std::string& url;
    std::string_view& raw_url;
    cc::query_string& params;
    double& USE_MAX_MEM_SIZE_MB;
#ifdef _WIN32
    long long length;
#else
    long length;
#endif // _WIN32
    str_map& headers;
    Conn& fiber;
    std::unique_ptr<fc::cache_file>& cache_file;
    std::unordered_map<std::string_view, std::string_view>& cookie_map;
    std::string_view body;
    void setTimeoutSec(std::function<void()>&& func, uint32_t seconds = 1);
    void setTimeout(std::function<void()>&& func, uint32_t milliseconds = 100);
  };// request

  class Res {
    fc::Ctx& Ctx;
    std::string& url;
    friend class fc::Conn;
    friend struct fc::App;
    enum algorithm { // 15 is the default value for deflate
      DEFLATE = 15, // windowBits can also be greater than 15 for optional gzip encoding.
      // Add 16 to windowBits to write a simple gzip header and trailer around the compressed data instead of a zlib wrapper.
      GZIP = 15 | 16,
    };
  public:
    fc::App& app;
    _FORCE_INLINE void set_content_type(const char* v) { Ctx.set_content_type(v, strlen(v)); };
    _FORCE_INLINE void set_content_type(const char* v, size_t l) { Ctx.set_content_type(v, std::move(l)); };
    _FORCE_INLINE void set_header(std::string_view&& k, std::string_view&& v) {
      Ctx.set_header(std::forward<std::string_view>(k), std::forward<std::string_view>(v));
    }
    _FORCE_INLINE void set_cookie(std::string_view k, std::string_view v) { Ctx.set_cookie(k, v); }
    _FORCE_INLINE Res(fc::Ctx& ctx, std::string& u, App* ap): Ctx(ctx), url(u), app(*ap) {}
    std::string body;
    int is_file{ 3 };
    uint16_t code{ 200 };// Check whether the response has a static file defined.
    //Generally used to read configuration files, or slow io operations, return Json
    void write_async(std::function<json::Json()>&& f, short i = CACHE_HTML_TIME_SECOND);
    //Generally used to read configuration files, or slow io operations, return string
    void write_async_s(std::function<std::string()>&& f, short i = CACHE_HTML_TIME_SECOND);
    _FORCE_INLINE void write(json::Json&& j) { Ctx.set_content_type("application/json", 16); Ctx.respond(j.str()); };
    _FORCE_INLINE void write(const json::Json& j) { Ctx.set_content_type("application/json", 16); Ctx.respond(j.str()); };
    _FORCE_INLINE void write(const std::string& b) { Ctx.set_content_type("text/plain;charset=UTF-8", 24); Ctx.respond(b); };
    _FORCE_INLINE void write(const std::string_view& b) { Ctx.respond(b); };
    _FORCE_INLINE void write(const char* b) { Ctx.respond(b); };
    void write(std::string&& body_part);
    inline void set_status(int s) { Ctx.set_status(s); }
    std::string& compress_str(char* const str, unsigned int len);
    std::string& decompress_str(char* const str, unsigned int len);
  };// response
}

#endif // REQRES_HH
