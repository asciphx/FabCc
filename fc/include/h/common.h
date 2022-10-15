#ifndef COMMON_H
#define COMMON_H
#include <string>
#include <hpp/string_view.hpp>
#if defined(_MSC_VER) || defined(_WIN32)
#include <locale.h>
#define WIN32_LEAN_AND_MEAN
#define _INLINE __forceinline
#else
#define _INLINE __attribute__((always_inline))
#endif
#define TYPE_GET(t, ptr, member) (t*)(ptr)-((size_t)&reinterpret_cast<char const volatile&>(((t*)0)->member))
#include <str_map.hh>
#include <h/config.h>
#include <str.hh>
namespace fc {
  static const fc::Buf RES_CT("Content-Type", 12), RES_CL("Content-Length", 14), RES_CALLBACK("CB", 2), RES_empty("", 0),
	RES_Loc("Location", 8), RES_Ca("Cache-Control", 13), RES_Cookie("Cookie", 6), RES_AJ("application/json", 16), RES_AR("Accept-Ranges", 13),
	RES_No("nosniff", 7), RES_Txt("text/html;charset=UTF-8", 23), RES_Con("Connection", 10), RES_Ex("expect", 6),
	RES_Xc("X-Content-Type-Options", 22), RES_Allow("Allow", 5), RES_CR("Content-Ranges", 14), RES_Range("Range", 5);
  static const fc::Buf& get_header(const str_map& headers, const fc::Buf& key) {
	if (headers.count(key)) { return headers.find(key)->second; } return RES_empty;
  }
  enum class HTTP { INVALID, GET, POST, PUT, DEL, HEAD, OPTIONS };
 // enum class HTTP {
	//DEL = 0, GET, HEAD, POST = 3, PUT, OPTIONS = 6, /*PATCH = 28,*/ INVALID
 // };
  static const std::string_view RES_server_tag("Server: ", 8), RES_content_length_tag("Content-Length: ", 16), RES_http_status("HTTP/1.1 ", 9),
	RES_con("connection", 10), RES_S_C("Set-Cookie", 10), RES_upgrade("upgrade", 7), RES_oct("application/octet-stream", 24),
	RES_AcC("Access-Control-Allow-Credentials: ", 34), RES_t("true", 4), RES_AcM("Access-Control-Allow-Methods: ", 30), RES_host("Host", 4),
	RES_AcH("Access-Control-Allow-Headers: ", 30), RES_AcO("Access-Control-Allow-Origin: ", 29), RES_expect("HTTP/1.1 100 Continue\r\n\r\n", 25),
	RES_date_tag("Date: ", 6), RES_content_length("content-length", 14), RES_seperator(": ", 2), RES_crlf("\r\n", 2), RES_loc("location", 8),
	RES_AE("Accept-Encoding", 15), RES_CE("Content-Encoding", 16), RES_gzip("gzip", 4), RES_deflate("deflate", 7), RES_bytes("bytes", 5),
	expect_100_continue("HTTP/1.1 100 Continue\r\n\r\n", 25);
  static const char RES_GMT[26] = "%a, %d %b %Y %H:%M:%S GMT";
  static std::string directory_ = STATIC_DIRECTORY;
  static std::string upload_path_ = UPLOAD_DIRECTORY;
}

#endif // COMMON_H