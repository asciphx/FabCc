#include <hh/http_error.hh>
namespace fc {
  http_error::http_error(int status, const std::string& what): status_(status), what_(what) {}
  http_error::http_error(int status, const char* what): status_(status), what_(what, strlen(what)) {}
  int http_error::i() const { return status_; }
  const char* http_error::what() const { return what_.c_str(); }
} // namespace fc
