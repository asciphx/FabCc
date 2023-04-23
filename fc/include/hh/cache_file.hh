/**
*
*  cache_file.h
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
#pragma once
#include <string>
#include <stdio.h>
#include <hpp/string_view.hpp>
namespace fc {
  class cache_file {
    char* data();
    size_t length();
    FILE* file_{ nullptr };
    bool autoDelete_{ false };
    const std::string path_;
    char* data_{ nullptr };
    size_t dataLength_{ 0 };
    cache_file(const cache_file&) = delete;
    void operator=(const cache_file&) = delete;
    cache_file(cache_file &&) noexcept(true) = default;
    cache_file &operator=(cache_file &&) noexcept(true) = default;
  public:
    explicit cache_file(const char* path, size_t l, bool autoDelete = true);
    ~cache_file();
    void append(const std::string& data) { append(data.data(), data.length()); }
    void append(const char* data, size_t length);
    std::string_view getStringView() { if (data()) return std::string_view(data_, dataLength_); return std::string_view(); }
  };
}
