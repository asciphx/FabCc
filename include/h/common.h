#ifndef COMMON_H
#define COMMON_H
#include <string>
#include <str_map.hh>
#include <str.hh>
namespace fc {
  //static std::string_view expect_100_continue("HTTP/1.1 100 Continue\r\n\r\n", 25);
  enum class HTTP {
	DEL = 0, GET, HEAD, POST, PUT, CONNECT, OPTIONS, TRACE, PATCH = 28, PURGE, InternalMethodCount
  };
#if defined(_MSC_VER)
#define _INLINE __forceinline
#else
#define _INLINE inline
#endif
  _INLINE std::string m2s(HTTP m) {
	switch (m) {
	case HTTP::DEL:return "DELETE";
	case HTTP::GET:return "GET";
	case HTTP::HEAD:return "HEAD";
	case HTTP::POST:return "POST";
	case HTTP::PUT:return "PUT";
	case HTTP::CONNECT:return "CONNECT";
	case HTTP::OPTIONS:return "OPTIONS";
	case HTTP::TRACE:return "TRACE";
	case HTTP::PATCH:return "PATCH";
	case HTTP::PURGE:return "PURGE";
	default:return "invalid";
	}
	return "invalid";
  }
  _INLINE HTTP c2m(const char* m) {
	switch (hack8Str(m)) {
	case "DELETE"_l:return HTTP::DEL;
	case 4670804:return HTTP::GET;
	case 1212498244:return HTTP::HEAD;
	case 1347375956:return HTTP::POST;
	case 5264724:return HTTP::PUT;
	case "CONNECT"_l:return HTTP::CONNECT;
	case "OPTIONS"_l:return HTTP::OPTIONS;
	case "TRACE"_l:return HTTP::TRACE;
	case "PATCH"_l:return HTTP::PATCH;
	case "PURGE"_l:return HTTP::PURGE;
	}
	return HTTP::InternalMethodCount;
  }

  static const std::string RES_CT("Content-Type", 12), RES_CL("Content-Length", 14), RES_CALLBACK("CB", 2), empty,
	RES_Loc("Location", 8), Res_Ca("Cache-Control", 13), RES_Cookie("Cookie", 6), RES_AJ("application/json", 16),
	RES_No("nosniff", 7), RES_Txt("text/html;charset=UTF-8", 23), RES_Xc("X-Content-Type-Options", 22), RES_Allow("Allow", 5);
#undef _INLINE
}
#endif // COMMON_H