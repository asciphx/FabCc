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
#ifdef _WIN32
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinNls.h>
inline char* UnicodeToUtf8(const char* str) {
  LPCSTR pszSrc = str;
  int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
  if (nLen == 0) return "";
  wchar_t* pwszDst = new wchar_t[nLen];
  MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
  std::wstring wstr(pwszDst); delete[] pwszDst; pwszDst = nullptr;
  const wchar_t* unicode = wstr.c_str();
  nLen = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
  char* szUtf8 = (char*)malloc(nLen + 1);
  memset(szUtf8, 0, nLen + 1);
  WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, nLen, NULL, NULL);
  return szUtf8;
}
#else
#include <stdlib.h>
#include <cstring>
inline char* UnicodeToUtf8(const char* str) {
  if (NULL == str) return NULL;
  size_t destlen = mbstowcs(0, str, 0);
  size_t size = destlen + 1;
  wchar_t* pw = new wchar_t[size];
  mbstowcs(pw, str, size);
  size = wcslen(pw) * sizeof(wchar_t);
  char* pc = (char*)malloc(size + 1); memset(pc, 0, size + 1);
  destlen = wcstombs(pc, pw, size + 1);
  pc[size] = 0; delete[] pw; pw = nullptr; return pc;
}
#endif
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
