#ifndef FILE_SPTR_HH
#define FILE_SPTR_HH
#include <memory>
#include <functional>
#include <string>
//from https://github.com/neithern/libhttp/blob/master/src/file-map.cpp
namespace fc {
  class file_sptr: public std::enable_shared_from_this<file_sptr> {
  public:
	file_sptr(const std::string& path, size_t length = 0, long modified_time = 0);
	~file_sptr();
	int read_chunk(int64_t offset, size_t size, std::function<void(const char* data, size_t size, std::function<void()> done)> sink);
	char* ptr_;
	size_t size_;
	long modified_time_;
  };

} // namespace fc

#endif // FILE_S_PTR_HH