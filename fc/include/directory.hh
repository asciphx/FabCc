#pragma once
#include <stdlib.h>
#include <string>
#include <direct.h>
#include <h/config.h>
#if defined _WIN32
#include <windows.h>
#define CROSSPLATFORM_MAX_PATH MAX_PATH
#else
#define CROSSPLATFORM_MAX_PATH PATH_MAX
#endif
namespace fc {
  bool crossplatform_realpath(const std::string& path, char out_buffer[CROSSPLATFORM_MAX_PATH]);
  int create_directory(const std::string& path);
  bool is_regular_file(const std::string& path);
  bool is_directory(const std::string& path);
  bool starts_with(const char* pre, const char* str);
} // namespace fc
