#ifndef FILE_SPTR_HH
#define FILE_SPTR_HH
#include <memory>
#include <functional>
#include <string>
namespace fc {
#ifdef _WIN32
#define _Fsize_t long long
#else
#define _Fsize_t long
#endif
  //Because the windows system does not have a sendfile method, this class may only be used for _WIN32
  class file_sptr: public std::enable_shared_from_this<file_sptr> {
  public:
    file_sptr(const std::string& path, _Fsize_t length = 0, long long modified_time = 0);
    ~file_sptr();
    int read_chunk(int64_t& o, int l, std::function<void(const char* c, int l, std::function<void()> f)> sink);
    char* ptr_;
    _Fsize_t size_;
    long long modified_time_;
  };

} // namespace fc

#endif // FILE_S_PTR_HH