#ifndef UTILS_HPP
#define UTILS_HPP
#ifdef _WIN32
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinNls.h>
#include <string>
inline char* UnicodeToUtf8(const char* str) {
  int nLen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
  if (nLen == 0) return NULL;
  wchar_t* pwszDst = new wchar_t[nLen];
  MultiByteToWideChar(CP_ACP, 0, str, -1, pwszDst, nLen);
  std::wstring wstr(pwszDst, nLen); delete[] pwszDst; pwszDst = nullptr;
  const wchar_t* unicode = wstr.c_str();
  nLen = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
  char* szUtf8 = (char*)malloc(nLen + 1);
  memset(szUtf8, 0, nLen + 1);
  WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, nLen, NULL, NULL);
  return szUtf8;
}
#else
#include <stdlib.h>
#include <wchar.h>
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

#endif // !UTILS_HPP
