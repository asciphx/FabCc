#ifndef FILE_SPTR_HH
#define FILE_SPTR_HH
#include <memory>
#include <functional>
#include <string>
#include "tp/c++.h"
#include "tp/ctx.hh"
namespace fc {
#ifdef _WIN32
#define _Fsize_t long long
#define _Fhandle void*
#else
#define _Fsize_t long
#define _Fhandle int
#endif
  class file_sptr: public std::enable_shared_from_this<file_sptr> {
  public:
    file_sptr(const std::string& path, _Fsize_t length = 0, long long modified_time = 0);
    file_sptr();
    ~file_sptr();
    _Fsize_t size_;
    long long modified_time_;
    _Fhandle fd_;
    _CTX_TASK(void) read_chunk(std::function<_CTX_TASK(void)(_Fhandle fd)> sink);
  };
} // namespace fc
#endif // FILE_S_PTR_HH