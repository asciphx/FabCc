#ifndef STR_HH
#define STR_HH
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stdint.h>
#include <buf.hh>
#if (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
#define unlikely(x) __builtin_expect(x, 0)
#define likely(x) __builtin_expect(x, 1)
#else
#define unlikely(x) x
#define likely(x) x
#endif
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
typedef signed char  i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
namespace str {
  template <typename T>
  inline void del(T* p) {
	if (p) { p->~T(); ::free((void*)p); }
  }
  template <typename T, typename... Args>
  inline T* make(Args&&... args) {
	return new (malloc(sizeof(T))) T(std::forward<Args>(args)...);
  }
  template<typename T>
  inline fc::Buf from(T&& t) {
	fc::Buf s(0x18); s << std::forward<T>(t); return s;
  }
  inline int _Shift(char c) {
	switch (c) {
	case 'k':
	case 'K':
	  return 10;
	case 'm':
	case 'M':
	  return 20;
	case 'g':
	case 'G':
	  return 30;
	case 't':
	case 'T':
	  return 40;
	case 'p':
	case 'P':
	  return 50;
	default:
	  return 0;
	}
  }
  inline bool to_bool(const char* s) {
	if (strcmp(s, "false") == 0 || strcmp(s, "0") == 0) return false;
	if (strcmp(s, "true") == 0 || strcmp(s, "1") == 0) return true;
	return false;
  }
  i64 to_int64(const char* s);
  i32 to_int32(const char* s);
  u64 to_uint64(const char* s);
  u32 to_uint32(const char* s);
  double to_double(const char* s);
  fc::Buf strip(const char* s, const char* c, char d = 'b');
  fc::Buf strip(const fc::Buf& s, const char* c = " \t\r\n", char d = 'b');
  std::vector<fc::Buf> split(const fc::Buf& s, char c, u32 maxsplit = 0);
  fc::Buf replace(const char* s, const char* sub, const char* to, u32 maxreplace = 0);
  fc::Buf replace(const fc::Buf& s, const char* sub, const char* to, u32 maxreplace = 0);
}
//<Ctrl> + Left mouse button -> Jump to the specified location
namespace fc {
  //RFC_ALL
  std::string DecodeURL(std::string& str);
  std::string DecodeURL(const char*& s, size_t l);
  std::string DecodeURL(const char* d);
  //RFC3986
  std::string EncodeURL(const std::string& s);
  std::string EncodeURL(const char* c);
  //RFC2396
  std::string EncodeURLComponent(const std::string& s);
  std::string EncodeURLComponent(const char* c);
  std::string& toUpperCase(std::string& s);
  std::string toUpperCase(const char* s);
  std::string& toLowerCase(std::string& s);
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
	inline unsigned long long hack8Str(const char* s) {
	  unsigned long long r = s[0]; for (signed char i = 0; ++i < 8 && s[i]; r <<= 8, r += s[i]); return r;
	}//If only the first four digits need to be matched and there is no conflict, it is recommended to use hack4Str to improve efficiency
	inline int hack4Str(const char* s) { int r = s[0]; for (signed char i = 0; ++i < 4 && s[i]; r <<= 8, r += s[i]); return r; }
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
#endif
