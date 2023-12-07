#ifndef HTTP_ERROR_HH
#define HTTP_ERROR_HH
#include <string>
#include <cstring>
namespace fc {
  struct http_error {
    http_error(int status, const std::string& what);
    http_error(int status, const char* what);
    int i() const;
    const char* what() const;
  private:
    int status_; std::string what_;
  };
  namespace err {
#define FC_HTTP_ERROR(CODE, ERR) static http_error ERR(const char* w = "") { return http_error(CODE, w); }\
 static http_error ERR(const std::string& w) { return http_error(CODE, w); }
    FC_HTTP_ERROR(400, bad_request)
      FC_HTTP_ERROR(401, unauthorized)
      FC_HTTP_ERROR(403, forbidden)
      FC_HTTP_ERROR(404, not_found)
      FC_HTTP_ERROR(413, too_large)
      FC_HTTP_ERROR(500, internal_server_error)
      FC_HTTP_ERROR(501, not_implemented)
      FC_HTTP_ERROR(510, not_extended)
#undef FC_HTTP_ERROR
  } // namespace err
} // namespace fc
#endif