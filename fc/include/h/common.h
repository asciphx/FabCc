#ifndef COMMON_H
#define COMMON_H
#include <string>
#include <chrono>
#include <hpp/string_view.hpp>
#include <hh/str_map.hh>
#include <h/config.h>
#include <str.hh>
namespace fc {
  static const std::string_view RES_CT("Content-Type", 12), RES_CL("Content-Length", 14), RES_CALLBACK("CB", 2), RES_Loc("Location", 8),
    RES_Ca("Cache-Control", 13), RES_Cookie("Cookie", 6), RES_AJ("application/json", 16), RES_AR("Accept-Ranges", 13), RES_NIL("", 1),
    RES_No("nosniff", 7), RES_Txt("text/html;charset=UTF-8", 23), RES_Con("Connection", 10), RES_Ex("expect", 6),
    RES_Xc("X-Content-Type-Options", 22), RES_Allow("Allow", 5), RES_CR("Content-Ranges", 14), RES_Range("Range", 5), RES_empty("", 0);
  enum class HTTP {
    DEL = 0, GET, HEAD, POST, PUT, OPTIONS, PATCH, CONNECT, INVALID
  };
  static const std::string_view RES_server_tag("Server: ", 8), RES_content_length_tag("Content-Length: ", 16), RES_http_status("HTTP/1.1 ", 9),
    RES_con("connection", 10), RES_S_C("Set-Cookie", 10), RES_upgrade("upgrade", 7), RES_oct("application/octet-stream", 24),
    RES_AcC("Access-Control-Allow-Credentials: ", 34), RES_t("true", 4), RES_AcM("Access-Control-Allow-Methods: ", 30), RES_host("Host", 4),
    RES_AcH("Access-Control-Allow-Headers: ", 30), RES_AcO("Access-Control-Allow-Origin: ", 29), RES_expect("HTTP/1.1 100 Continue\r\n\r\n", 25),
    RES_date_tag("Date: ", 6), RES_content_length("content-length", 14), RES_seperator(": ", 2), RES_crlf("\r\n", 2), RES_loc("location", 8),
    RES_AE("Accept-Encoding", 15), RES_CE("Content-Encoding", 16), RES_gzip("gzip", 4), RES_deflate("deflate", 7), RES_bytes("bytes", 5),
    expect_100_continue("HTTP/1.1 100 Continue\r\n\r\n", 25);
  static std::string directory_ = STATIC_DIRECTORY;
  static std::string upload_path_ = UPLOAD_DIRECTORY;
  static std::string server_name_ = SERVER_NAME;
  static std::chrono::time_point<std::chrono::high_resolution_clock> RES_START_TIME;
}

#endif // COMMON_H