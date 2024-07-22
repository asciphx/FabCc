#ifndef STR_HH
#define STR_HH
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wempty-body"
#pragma clang diagnostic ignored "-Wswitch"
#endif
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stdint.h>
#include "hpp/string_view.hpp"
#include "tp/c++.h"
#define DISALLOW_COPY_AND_ASSIGN(T) T(const T&) = delete; void operator=(const T&) = delete
#ifdef _WIN32
namespace color {
  struct Color {
    Color(const char* s, int i);
    union { int i; const char* s; };
  };
  extern const Color red;
  extern const Color green;
  extern const Color blue;
  extern const Color yellow;
  extern const Color deflt; // default color
} // color
std::ostream& operator<<(std::ostream&, const color::Color&);
#else
namespace color {
  const char* const red = "\033[38;5;1m";
  const char* const green = "\033[38;5;2m";
  const char* const blue = "\033[38;5;12m";
  const char* const yellow = "\033[38;5;3m"; // or 11
  const char* const deflt = "\033[39m";
}
#endif
//<Ctrl> + Left mouse button -> Jump to the specified location
namespace fc {
  //RFC_ALL
  std::string DecodeURL(std::string& str);
  std::string_view DecodeURL(const char*& s, size_t l);
  std::string_view DecodeURL(const char* d);
  //RFC3986
  std::string EncodeURL(const std::string& s);
  std::string EncodeURL(const char* c);
  //RFC2396
  std::string EncodeURLComponent(const std::string& s);
  std::string EncodeURLComponent(const char* c);
  //toCase
  std::string& toUpperCase(std::string& s);
  std::string toUpperCase(const char* s);
  std::string& toLowerCase(std::string& s);
  std::string toLowerCase(std::string&& s);
  std::string toLowerCase(const char* s);
  std::string toSqlCase(const char* s);
  std::string toQuotes(const char* s);
  std::ostream& operator<<(std::ostream& os, const tm& _v);
  std::string& operator<<(std::string& s, const tm& _v);
  tm operator+(tm& t, tm& m);
  tm operator-(tm& t, tm& m);
  bool operator==(tm& t, tm& m);
  bool operator!=(tm& t, tm& m);
  bool operator<(tm& t, tm& m);
  bool operator>(tm& t, tm& m);
  bool operator<=(tm& t, tm& m);
  bool operator>=(tm& t, tm& m);
  template<typename T> inline const char* ObjName() {
    const char* s = typeid(T).name();
#ifdef _WIN32
    while (*++s != 0x20); return ++s;
#else
    while (*s < 0x3a && *s++ != 0x24) {}; return s;
#endif
  }
  template<typename T, typename U> inline uint64_t ObjLink() {//*s > 0x5e ? *s - 0x5f : 
    const char* s = typeid(T).name(), * c = typeid(U).name(); unsigned long long r = 0;
#ifdef _WIN32
    while (*++s != 0x20); while (*++c != 0x20); for (; *++s; r *= 0x17, r += *s > 0x5c ? *s - 0x5d : *s - 0x12);
    for (; *++c; r *= 0x17, r += *c > 0x5c ? *c - 0x5d : *c - 0x12);
#else
    while (*s < 0x3a && *s++ != 0x24) {}; while (*c < 0x3a && *c++ != 0x24) {};
    for (; *s; r *= 0x17, r += *s > 0x5c ? *s - 0x5d : *s - 0x12, ++s);
    for (; *c; r *= 0x17, r += *c > 0x5c ? *c - 0x5d : *c - 0x12, ++c);
#endif
    return r;
  }
#ifdef __cplusplus
  extern "C" {
#endif
    long long strLen(const char* s);
    void strCpy(char* d, const char* s);
    char* strStr(char* d, const char* s);
    int strCmp(const char* c, const char* s);
    //If not safe, _f
    char* subStr_f(const char* c, int i, int e);
    char* to8Str_f(unsigned long long i);
    char* to4Str_f(int i);
    inline unsigned long long hack_str(const char* s, size_t l) { unsigned long long r = *s; for (; --l; r <<= 8, r += *++s); return r; }
    //Hack8str is downward compatible with hack4str, however, it is not compatible with the hackstr method
    inline unsigned long long hackStr(const char* s) {
      unsigned long long r = s[0] > 0x5c ? s[0] - 0x5d : s[0] - 0x12;
      for (unsigned short i = 0; s[++i]; r *= 0x17, r += s[i] > 0x5c ? s[i] - 0x5d : s[i] - 0x12); return r;
    }
    inline unsigned long long hackUrl(const char* s) {
      unsigned long long r = s[0] - 0x23;
      for (unsigned char i = 0; s[++i]; r *= 0x29, r += s[i] > 0x5e ? s[i] - 0x5f : s[i] > 0x40 ? s[i] - 0x3f : s[i] - 0x10); return r;
    }//s[++i] > 0x22
#ifdef __cplusplus
  }  /* extern "C" */
#endif
}
#if __clang__
#pragma clang diagnostic pop
#endif
#endif
