/**
*
*  cache_file.cc
*  An Tao
*
*  Copyright 2018, An Tao.  All rights reserved.
*  https://github.com/an-tao/drogon
*  Use of this source code is governed by a MIT license
*  that can be found in the License file.
*
*  Drogon
*
*/
#include "hh/cache_file.hh"
#ifdef _WIN32
#include "h/mman.h"
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include "hpp/utils.hpp"
using namespace fc;
cache_file::cache_file(const char* path, size_t l, bool autoDelete)
  : autoDelete_(autoDelete)
#ifndef _MSC_VER
  , path_(path, l) {
  file_ = fopen(path_.data(), "wb+");
#else
  {
    char* c = UnicodeToUtf8(path); file_ = fopen(c, "wb+"); path_ = c; free(c);
#endif
  }
cache_file::~cache_file() {
  if (data_) {
    munmap(data_, dataLength_);
  }
  if (autoDelete_ && file_) {
    fclose(file_);
#if defined(_WIN32) && !defined(__MINGW32__)
    _unlink(path_.c_str());
#else
    unlink(path_.data());
#endif
  } else if (file_) {
    fclose(file_);
  }
}
void cache_file::append(const char* data, size_t length) {
  if (file_)
    fwrite(data, length, 1, file_);
}
size_t cache_file::length() {
  if (file_)
#ifdef _WIN32
    return _ftelli64(file_);
#else
    return ftell(file_);
#endif
  return 0;
}
char* cache_file::data() {
  if (!file_)
    return nullptr;
  if (!data_) {
    fflush(file_);
#ifdef _WIN32
    auto fd = _fileno(file_);
#else
    auto fd = fileno(file_);
#endif
    dataLength_ = length();
    data_ = static_cast<char*>(
      mmap(nullptr, dataLength_, PROT_READ, MAP_SHARED, fd, 0));
    if (data_ == MAP_FAILED) {
      data_ = nullptr;
    }
  }
  return data_;
}
