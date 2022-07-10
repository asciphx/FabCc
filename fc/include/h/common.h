#ifndef COMMON_H
#define COMMON_H
#include <string>
#if _HAS_CXX17==0 || !defined(_HAS_CXX17)
#    define string_view string
#else
#include <string_view>
#endif
#include <str_map.hh>
#include <str.hh>
#if defined(_MSC_VER)
#define _INLINE __forceinline
#else
#define _INLINE inline
#endif
#if _DEBUG
#    define DEBUG printf
#else
#    define DEBUG(...)
#endif
#define TYPE_GET(t, ptr, member) (t*)(ptr)-((size_t)&reinterpret_cast<char const volatile&>(((t*)0)->member))
namespace fc {
  static const std::string RES_CT("Content-Type", 12), RES_CL("Content-Length", 14), RES_CALLBACK("CB", 2), empty,
	RES_Loc("Location", 8), RES_Ca("Cache-Control", 13), RES_Cookie("Cookie", 6), RES_AJ("application/json", 16),
	RES_No("nosniff", 7), RES_Txt("text/html;charset=UTF-8", 23), RES_Xc("X-Content-Type-Options", 22), RES_Allow("Allow", 5);
  static const std::string& get_header(const str_map& headers, const std::string& key) {
	if (headers.count(key)) { return headers.find(key)->second; } return empty;
  }
  static const std::string_view expect_100_continue("HTTP/1.1 100 Continue\r\n\r\n", 25);
  enum class HTTP {
	DEL = 0, GET, POST = 3, PUT, PATCH = 28, InternalMethodCount
  };
  _INLINE const char* m2c(HTTP m) {
	switch (m) {
	case HTTP::DEL:return "DELETE";
	case HTTP::GET:return "GET";
	//case HTTP::HEAD:return "HEAD";
	case HTTP::POST:return "POST";
	case HTTP::PUT:return "PUT";
	//case HTTP::CONNECT:return "CONNECT";
	//case HTTP::OPTIONS:return "OPTIONS";
	//case HTTP::TRACE:return "TRACE";
	case HTTP::PATCH:return "PATCH";
	//case HTTP::PURGE:return "PURGE";
	default:return "invalid";
	}
	return "invalid";
  }
  static const std::string RES_server_tag("Server: ", 8), RES_content_length_tag("Content-Length: ", 16), RES_http_status("HTTP/1.1 ", 9),
	RES_Con("Connection", 10), RES_con("connection", 10), RES_S_C("Set-Cookie", 10), RES_Ex("expect", 6), RES_upgrade("upgrade", 7),
	RES_AcC("Access-Control-Allow-Credentials: ", 34), RES_t("true", 4), RES_AcM("Access-Control-Allow-Methods: ", 30), RES_host("Host", 4),
	RES_AcH("Access-Control-Allow-Headers: ", 30), RES_AcO("Access-Control-Allow-Origin: ", 29), RES_expect("HTTP/1.1 100 Continue\r\n\r\n", 25),
	RES_date_tag("Date: ", 6), RES_content_length("content-length", 14), RES_seperator(": ", 2), RES_crlf("\r\n", 2), RES_loc("location", 8),
	RES_AE("Accept-Encoding", 15), RES_CE("Content-Encoding", 16), RES_gzip("gzip", 4), RES_deflate("deflate", 7);
}
#endif // COMMON_H