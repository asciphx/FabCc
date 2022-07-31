#include <http_error.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/error.hh
namespace fc {
  http_error::http_error(int status, const Buffer& what): status_(status), what_(what) {}
  http_error::http_error(int status, const char* what): status_(status), what_(what, (unsigned int)strlen(what)) {}
  int http_error::i() const { return status_; }
  const std::string_view http_error::what() const { return what_.data(); }
} // namespace fc
