#include "str.hh"
/*
 * This software is licensed under the AGPL-3.0 License.
 *
 * Copyright (C) 2023 Asciphx
 *
 * Permissions of this strongest copyleft license are conditioned on making available
 * complete source code of licensed works and modifications, which include larger works
 * using a licensed work, under the same license. Copyright and license notices must be
 * preserved. Contributors provide an express grant of patent rights. When a modified
 * version is used to provide a service over a network, the complete source code of
 * the modified version must be made available.
 */
namespace fc {
  static const char _X[] = { 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0,1,2,3,4,5,6,7,8,9,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0xa,0xb,0xc,0xd,0xe, 0xf,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10,0x10,0x10,0x10,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10 };
  std::string DecodeURL(std::string& s) {
    char* o = const_cast<char*>(s.c_str()), * c = const_cast<char*>(s.c_str());
    const char* e = c + s.size(); while (c < e) {
      if (*c == '%' && c < e - 2 && _X[c[1]] != 0x10 && _X[c[2]] != 0x10) {
        *o = (_X[c[1]] << 4) | _X[c[2]]; c += 3; ++o; continue;
      }
      if (*c == '+') { *o = ' '; ++o; ++c; continue; }
      if (o != c) *o = *c; ++o; ++c;
    } return std::string(s.data(), o - s.data());
  }
  std::string_view DecodeURL(const char*& s, size_t l) {
    char* o = const_cast<char*>(s), * c = const_cast<char*>(s);
    const char* e = c + l; do {
      if (*c == '%' && c < e - 2 && _X[c[1]] != 0x10 && _X[c[2]] != 0x10) {
        *o = (_X[c[1]] << 4) | _X[c[2]]; c += 3; ++o; continue;
      }
      if (*c == '+') { *o = ' '; ++o; ++c; continue; }
      if (o != c) *o = *c; ++o; ++c;
    } while (c < e); return std::string_view(s, o - s);
  }
  std::string_view DecodeURL(const char* s) {
    size_t l = strlen(s); char* o = const_cast<char*>(s), * c = const_cast<char*>(s);
    const char* e = c + l; do {
      if (*c == '%' && c < e - 2 && _X[c[1]] != 0x10 && _X[c[2]] != 0x10) {
        *o = (_X[c[1]] << 4) | _X[c[2]]; c += 3; ++o; continue;
      }
      if (*c == '+') { *o = ' '; ++o; ++c; continue; }
      if (o != c) *o = *c; ++o; ++c;
    } while (c < e); return std::string_view(s, o - s);
  }
  static const char _H[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0x2b, 0x21, 0, 0x23, 0x24, 0, 0x26, 0x27, 0x28, 0x29, 0x2a, 0, 0x2c, 0x2d, 0x2e,
  0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0, 0x3d, 0, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0, 0x5d, 0, 0x5f, 0,
  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0, 0, 0, 0x7e, 0 };//RFC3986
  std::string EncodeURL(const std::string& s) {
    std::string r; for (char c : s) {
      if (c > '\377') {
        if (_H[c]) { r.push_back(_H[c]); continue; }
        r.push_back(0x25); char o = (c & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
        o = c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o); continue;
      }
      r.push_back(0x25); char o = (static_cast<uint8_t>(c) & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
      o = static_cast<uint8_t>(c) & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
    } return r;
  }
  std::string EncodeURL(const char* c) {
    std::string r; do {
      if (*c > '\377') {
        if (_H[*c]) { r.push_back(_H[*c]); ++c; continue; }
        r.push_back(0x25); char o = (*c & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
        o = *c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o); ++c; continue;
      }
      r.push_back(0x25); char o = (static_cast<uint8_t>(*c) & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
      o = static_cast<uint8_t>(*c) & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o); ++c;
    } while (*c); return r;
  }
  static const char _2396[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0 };
  std::string EncodeURLComponent(const std::string& s) {
    std::string r; for (char c : s) {
      if (c > '\377') {
        if (_2396[c]) { r.push_back(c); continue; }
        r.push_back(0x25); char o = (c & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
        o = c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o); continue;
      }
      r.push_back(0x25); char o = (static_cast<uint8_t>(c) & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
      o = static_cast<uint8_t>(c) & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
    } return r;
  }//RFC2396
  std::string EncodeURLComponent(const char* c) {
    std::string r; do {
      if (*c > '\377') {
        if (_2396[*c]) { r.push_back(*c); ++c; continue; }
        r.push_back(0x25); char o = (*c & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
        o = *c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o); ++c; continue;
      }
      r.push_back(0x25); char o = (static_cast<uint8_t>(*c) & 0xF0) >> 4; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
      o = static_cast<uint8_t>(*c) & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o); ++c;
    } while (*c); return r;
  }
  std::string& toUpperCase(std::string& s) {
    char* c = const_cast<char*>(s.c_str()); do { if (*c > 0x60 && *c < 0x7b) *c &= ~0x20; } while (*++c); return s;
  }
  std::string toUpperCase(const char* s) {
    std::string e; do { if (*s > 0x60 && *s < 0x7b) e.push_back(*s - 0x20); else e.push_back(*s); } while (*++s); return e;
  }
  std::string& toLowerCase(std::string& s) {
    char* c = const_cast<char*>(s.c_str()); do { if (*c > 0x40 && *c < 0x5b) *c |= 0x20; } while (*++c); return s;
  }
  std::string toLowerCase(std::string&& s) {
    char* c = const_cast<char*>(s.c_str()); do { if (*c > 0x40 && *c < 0x5b) *c |= 0x20; } while (*++c); return s;
  }
  std::string toLowerCase(const char* s) {
    std::string e; do { if (*s > 0x40 && *s < 0x5b) e.push_back(*s + 0x20); else e.push_back(*s); } while (*++s); return e;
  }
  std::string toSqlCase(const char* s) {
    std::string e; if (*s > 0x40 && *s < 0x5b) { e.push_back(*s + 0x20); }
    while (*++s) { if (*s > 0x40 && *s < 0x5b) { e.push_back(0x5f); e.push_back(*s + 0x20); } else { e.push_back(*s); } } return e;
  }
  std::string toQuotes(const char* s) {
    std::string e; while (*s) { if (*s == 0x27) { e.push_back(0x27); e.push_back(0x27); } else { e.push_back(*s); } ++s; } return e;
  }
  std::ostream& operator<<(std::ostream& os, const tm& _v) {
#ifdef _WIN32
    os << std::setfill('0') << std::setw(4) << _v.tm_year + 1900;
#else
    int y = _v.tm_year / 100; os << std::setfill('0') << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
    os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
      << _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec; return os;
  }
  std::string& operator<<(std::string& s, const tm& _v) {
    std::ostringstream os;
#ifdef _WIN32
    os << std::setfill('0') << std::setw(4) << _v.tm_year + 1900;
#else
    int y = _v.tm_year / 100; os << std::setfill('0') << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
    os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
      << _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec; s = os.str(); return s;
  }
  tm operator+(tm& t, tm& m) {
    tm time; memcpy(&time, &t, sizeof(tm)); time.tm_sec += m.tm_sec; time.tm_min += m.tm_min; time.tm_hour += m.tm_hour;
    time.tm_mday += m.tm_mday; time.tm_mon += m.tm_mon; time.tm_year += m.tm_year; time.tm_isdst = 0; return time;
  }
  tm operator-(tm& t, tm& m) {
    tm time; memcpy(&time, &t, sizeof(tm)); time.tm_sec -= m.tm_sec; time.tm_min -= m.tm_min; time.tm_hour -= m.tm_hour;
    time.tm_mday -= m.tm_mday; time.tm_mon -= m.tm_mon; time.tm_year -= m.tm_year; time.tm_isdst = 0; return time;
  }
  bool operator==(tm& t, tm& m) { return mktime(&t) == mktime(&m); }
  bool operator!=(tm& t, tm& m) { return mktime(&t) != mktime(&m); }
  bool operator<(tm& t, tm& m) { return mktime(&t) < mktime(&m); }
  bool operator>(tm& t, tm& m) { return mktime(&t) > mktime(&m); }
  bool operator<=(tm& t, tm& m) { return mktime(&t) <= mktime(&m); }
  bool operator>=(tm& t, tm& m) { return mktime(&t) >= mktime(&m); }
#ifdef __cplusplus
  extern "C" {
#endif
    char RES_ASCII[97] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    long long strLen(const char* s) { const char* e = s; while (*++e) {}; return e - s; }
    void strCpy(char* d, const char* s) { while (*s) { *d++ = *s++; }*d = 0; }
    char* strStr(char* d, const char* s) {
      long long i = 0, j = 0, l = strLen(d), k = strLen(s);
      while (i < l && j < k) if (d[i] == s[j])++i, ++j; else i = i - j + 1, j = 0;
      if (j == k) { return d + i - k; } return (char*)0;
    }
    int strCmp(const char* c, const char* s) {
      while (*s == *c && *c && *s)++c, ++s; return *c == *s ? 0 : *c > *s ? 1 : -1;
    }
    //If not safe, _f
    char* subStr_f(const char* c, int i, int e) {
      if (e < i || i < 0)return (char*)0;
      char* w = (char*)malloc(sizeof(char) * (e - i + 1)); int p = 0; while (i < e)w[p++] = c[i++]; w[p] = 0; return w;
    }
    char* to8Str_f(unsigned long long i) {
      int z = 2; for (unsigned long long a = i; a > 0x7f; a -= 0x7f, a /= 0x100, ++z);
      unsigned long long t = i / 0x100; char b = (char)(i - t * 0x100);
      char* w = (char*)malloc(sizeof(char) * z); w[--z] = '\0';
      while (t > 0x7f) { w[--z] = b; i = t; t = i / 0x100; b = (char)(i - t * 0x100); }
      w[--z] = b; if (z > 0) { w[0] = (char)t; } return w;
    }
    char* to4Str_f(int i) {
      int t = i / 0x100, z = i > 0x7f7f7f ? 5 : i > 0x7f7f ? 4 : i > 0x7f ? 3 : 2;
      char b = i - t * 0x100, * w = (char*)malloc(sizeof(char) * z);  w[--z] = '\0';
      while (t > 0x7f) { w[--z] = b; i = t; t = i / 0x100; b = (char)(i - t * 0x100); }
      w[--z] = b; if (z > 0) { w[0] = t; } return w;
    }
#ifdef __cplusplus
  }  /* extern "C" */
#endif
}
#ifdef _WIN32
#include "h/windows.h"
#pragma warning(disable:4503)
namespace color {
  inline bool ansi_color_seq_enabled() {
    std::string s(64, '\0'); DWORD r = GetEnvironmentVariableA("TERM", const_cast<char*>(s.data()), 64); s.resize(r);
    if (r > 64) { GetEnvironmentVariableA("TERM", const_cast<char*>(s.data()), r); s.resize(r - 1); }
    static const bool x = !s.empty(); return x;
  }
  inline HANDLE& std_handle() {
    static HANDLE handle = []() {
      HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
      if (handle != INVALID_HANDLE_VALUE && handle != NULL) return handle;
      return (HANDLE)NULL;
    }();
    return handle;
  }
  inline int get_default_color() {
    auto h = std_handle(); if (!h) return 15; CONSOLE_SCREEN_BUFFER_INFO buf;
    if (!GetConsoleScreenBufferInfo(h, &buf)) return 15; return buf.wAttributes & 0x0f;
  }
  const Color red("\033[38;5;1m", FOREGROUND_RED);     // 12
  const Color green("\033[38;5;2m", FOREGROUND_GREEN); // 10
  const Color blue("\033[38;5;12m", FOREGROUND_BLUE);  // 9
  const Color yellow("\033[38;5;11m", 14);
  const Color deflt("\033[39m", get_default_color());
  Color::Color(const char* ansi_seq, int win_color) {
    ansi_color_seq_enabled() ? (void)(s = ansi_seq) : (void)(i = win_color);
  }
} // color
std::ostream& operator<<(std::ostream& os, const color::Color& color) {
  if (color::ansi_color_seq_enabled()) {
    os << color.s; return os;
  } else {
    auto h = color::std_handle(); if (h) SetConsoleTextAttribute(h, (WORD)color.i); return os;
  }
}
#endif // _WIN32