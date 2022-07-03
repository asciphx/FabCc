#include <ctime>
#include <string>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>

static char STD_HEX[0x7e] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
0, 0, 0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/url_unescape.hh
std::string_view DecodeURL(std::string& s) {
  char* o = (char*)s.c_str(), * c = (char*)s.c_str();
  const char* e = c + s.size();
  while (c < e) {
	if (*c == '%' && c < e - 2) {
	  *o = (STD_HEX[c[1]] << 4) | STD_HEX[c[2]]; c += 2;
	} else if (o != c) *o = *c; ++o; ++c;
  }
  return std::string_view(s.data(), o - s.data());// 0x67
}
std::string& toUpperCase(std::string& s) {
  char* c = (char*)s.c_str(); if (*c > 0x60 && *c < 0x7b) { *c &= ~0x20; }
  while (*++c) { if (*c > 0x60 && *c < 0x7b) *c &= ~0x20; } return s;
}
std::string toUpperCase(const char* s) {
  std::string e; if (*s > 0x60 && *s < 0x7b) { e.push_back(*s - 0x20); }
  while (*++s) { if (*s > 0x60 && *s < 0x7b) { e.push_back(*s - 0x20); } else { e.push_back(*s); } } return e;
}
std::string& toLowerCase(std::string& s) {
  char* c = (char*)s.c_str(); if (*c > 0x40 && *c < 0x5b) { *c |= 0x20; }
  while (*++c) { if (*c > 0x40 && *c < 0x5b) *c |= 0x20; } return s;
}
std::string toLowerCase(const char* s) {
  std::string e; if (*s > 0x40 && *s < 0x5b) { e.push_back(*s + 0x20); }
  while (*++s) { if (*s > 0x40 && *s < 0x5b) { e.push_back(*s + 0x20); } else { e.push_back(*s); } } return e;
}
std::string toSqlCase(const char* s) {
  std::string e; if (*s > 0x40 && *s < 0x5b) { e.push_back(*s + 0x20); }
  while (*++s) { if (*s > 0x40 && *s < 0x5b) { e.push_back(0x5f); e.push_back(*s + 0x20); } else { e.push_back(*s); } } return e;
}
std::string toQuotes(const char* s) {
  std::string e; while (*s) { if (*s == 0x27) { e.push_back(0x27); e.push_back(0x27); } else { e.push_back(*s); } *++s; } return e;
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
  int strLen(const char* s) { const char* e = s; while (*++e); return e - s; }
  void strCpy(char* d, const char* s) { while (*s) { *d++ = *s++; }*d = 0; }
  char* strStr(char* d, const char* s) {
	int i = 0, j = 0, l = strLen(d), k = strLen(s);
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
	unsigned long long b, t = i / 0x100; b = i - t * 0x100 - 32;
	char* w = (char*)malloc(sizeof(char) * z); w[--z] = '\0';
	while (t > 0x7f) { w[--z] = RES_ASCII[b]; i = t; t = i / 0x100; b = i - t * 0x100 - 32; }
	w[--z] = RES_ASCII[b]; if (z > 0) { t -= 32; w[0] = RES_ASCII[t]; } return w;
  }
  char* to4Str_f(int i) {
	int t = i / 0x100, b = i - t * 0x100 - 32, z = i > 0x7f7f7f ? 5 : i > 0x7f7f ? 4 : i > 0x7f ? 3 : 2;
	char* w = (char*)malloc(sizeof(char) * z);  w[--z] = '\0';
	while (t > 0x7f) { w[--z] = RES_ASCII[b]; i = t; t = i / 0x100; b = i - t * 0x100 - 32; }
	w[--z] = RES_ASCII[b]; if (z > 0) { t -= 32; w[0] = RES_ASCII[t]; } return w;
  }
  unsigned long long hack8Str(const char* s) {
	unsigned long long r = s[0]; for (signed char i = 0; ++i < 8 && s[i]; r << 8, r += s[i]); return r;
  }//If only the first four digits need to be matched and there is no conflict, it is recommended to use hack4Str to improve efficiency
  int hack4Str(const char* s) { int r = s[0]; for (signed char i = 0; ++i < 4 && s[i]; r << 8, r += s[i]); return r; }
  //Hack8str is downward compatible with hack4str, however, it is not compatible with the hackstr method
  unsigned long long hackStr(const char* s) {
	unsigned long long r = s[0] > 0x5c ? s[0] - 0x5d : s[0] - 0x12;
	for (unsigned short i = 0; s[++i]; r *= 0x17, r += s[i] > 0x5c ? s[i] - 0x5d : s[i] - 0x12); return r;
  }
  unsigned long long hackUrl(const char* s) {
	unsigned long long r = s[0] - 0x23;
	for (unsigned char i = 0; s[++i] > 0x22; r << 5, r += s[i] > 0x5e ? s[i] - 0x5f : s[i] > 0x40 ? s[i] - 0x3f : s[i] - 3); return r;
  }
#ifdef __cplusplus
}  /* extern "C" */
#endif
unsigned long long operator""_l(const char* s, size_t /*len*/) {
  unsigned long long r = s[0];  for (unsigned long long i = 0; ++i < 8 && s[i]; r << 8, r += s[i]); return r;
}
int operator""_i(const char* s, size_t /*len*/) {
  int r = s[0]; for (int i = 0; ++i < 4 && s[i]; r << 8, r += s[i]); return r;
}
//You can match more strings with hackstr method, but you need to match ""_a used together
unsigned long long operator""_a(const char* s, size_t /*len*/) {
  unsigned long long r = s[0] > 0x5c ? s[0] - 0x5d : s[0] - 0x12;
  for (unsigned long long i = 0; s[++i]; r *= 0x17, r += s[i] > 0x5c ? s[i] - 0x5d : s[i] - 0x12); return r;
}
