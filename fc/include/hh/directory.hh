#ifndef DIRECTORY_HH
#define DIRECTORY_HH
#include <stdlib.h>
#include <string>
#include <cstring>
#include <vector>
#if defined _WIN32
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#ifdef _MSVC_LANG
#undef stat64
#define stat64 _stat64
#else
#include <sys/stat.h>
#endif
#include <direct.h>
#include <windows.h>
#define CROSSPLATFORM_MAX_PATH MAX_PATH
#include <io.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#define CROSSPLATFORM_MAX_PATH PATH_MAX
#endif
namespace fc {
  struct dir_iter {
    std::string name;
    long long size;
    char ext[8] = { 0 };//extension
  };
  bool crossplatform_realpath(const std::string& path, char out_buffer[CROSSPLATFORM_MAX_PATH]);
  int create_directory(const std::string& path);
  bool is_regular_file(const std::string& path);
  bool is_regular_file(const char* path);
  bool is_directory(const std::string& path);
  bool is_directory(const char* path);
  bool exists(const char* path);
  bool starts_with(const char* pre, const char* str);
  std::vector<fc::dir_iter> directory_iterator(const std::string& p, bool sub = false, int n = 6);
} // namespace fc
#endif
