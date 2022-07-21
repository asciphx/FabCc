#include <directory.hh>
namespace fc {
  bool crossplatform_realpath(const std::string& path, char out_buffer[CROSSPLATFORM_MAX_PATH]) {
	// Check if file exists by real file path.
#if defined _WIN32
	return 0 != GetFullPathNameA(path.c_str(), CROSSPLATFORM_MAX_PATH, out_buffer, nullptr);
#else
	return nullptr != realpath(path.c_str(), out_buffer);
#endif
  }
  int create_directory(const std::string& path) {
#if defined _WIN32
#if _MSC_VER < 1900
	int ret = mkdir(path.c_str());
#else
	int ret = _mkdir(path.c_str());
#endif
#else
	int ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	if (ret && errno == EEXIST) {
	  DEBUG("\"%s\" aleardy exist\n", path.c_str()); return 1;
	} else if (ret) {
	  DEBUG("create error: %d, :%s\n", ret, strerror(errno)); return -1;
	}
	return 0;
  }
  bool is_regular_file(const std::string& path) {
	struct stat ps; if (-1 == stat(path.c_str(), &ps)) return false; return ps.st_mode & S_IFREG;
  }
  bool is_directory(const std::string& path) {
	struct stat ps; if (-1 == stat(path.c_str(), &ps)) return false; return ps.st_mode & S_IFDIR;
  }

  bool starts_with(const char* pre, const char* str) {
	size_t lp = strlen(pre), ls = strlen(str); return ls < lp ? false : memcmp(pre, str, lp) == 0;
  }
} // namespace fc
