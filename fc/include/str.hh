#ifndef STR_HH
#define STR_HH
#include <ctime>
#include <string>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <cstdlib>
//<Ctrl> + Left mouse button -> Jump to the specified location
namespace fc {
  //RFC_ALL
  std::string DecodeURL(std::string& str);
  //RFC_ALL
  std::string DecodeURL(const char* d);
  //RFC3986
  std::string EncodeURL(const std::string& s);
  //RFC3986
  std::string EncodeURL(const char* c);
  //RFC2396
  std::string EncodeURLComponent(const std::string& s);
  //RFC2396
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
  constexpr unsigned long long operator""_l(const char* s, size_t /*len*/) {
	unsigned long long r = s[0];  for (unsigned long long i = 0; ++i < 8 && s[i]; r << 8, r += s[i]); return r;
  }
  constexpr int operator""_i(const char* s, size_t /*len*/) {
	int r = s[0]; for (int i = 0; ++i < 4 && s[i]; r << 8, r += s[i]); return r;
  }
  //You can match more strings with hackstr method, but you need to match ""_a used together
  constexpr unsigned long long operator""_a(const char* s, size_t /*len*/) {
	unsigned long long r = s[0] > 0x5c ? s[0] - 0x5d : s[0] - 0x12;
	for (unsigned long long i = 0; s[++i]; r *= 0x17, r += s[i] > 0x5c ? s[i] - 0x5d : s[i] - 0x12); return r;
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
	  unsigned long long r = s[0]; for (signed char i = 0; ++i < 8 && s[i]; r << 8, r += s[i]); return r;
	}//If only the first four digits need to be matched and there is no conflict, it is recommended to use hack4Str to improve efficiency
	inline int hack4Str(const char* s) { int r = s[0]; for (signed char i = 0; ++i < 4 && s[i]; r << 8, r += s[i]); return r; }
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
//Basic judgment type characters supported by ccORM (with switch(hack8Str(...)))
#if _WIN32
#define T_INT8 "signed char"_l
#define T_UINT8 " char"_l
#define T_INT16 "short"_l
#define T_UINT16 " short"_l
#define T_INT 'int'
#define T_UINT " int"_l
#define T_INT64 "__int64"_l
#define T_UINT64 " __int64"_l
#define T_BOOL 'bool'
#define T_DOUBLE "double"_l
#define T_FLOAT "float"_l
#define T_TM "struct tm"_l
#define T_TEXT "class text"_l
#define T_STRING "class std::basic_string"_l
//#elif __APPLE__
#else
#define T_INT8 'a'
#define T_UINT8 'h'
#define T_INT16 's'
#define T_UINT16 't'
#define T_INT 'i'
#define T_UINT 'j'
#define T_INT64 'x'
#define T_UINT64 'y'
#define T_BOOL 'b'
#define T_DOUBLE 'd'
#define T_FLOAT 'f'
#define T_TM '2tm'
#define T_TEXT "4textILt"_l
#define T_STRING "NSt7__cx"_l
#endif
//Corrected type character sequence
#define T_INT8_ 'a'
#define T_UINT8_ 'h'
#define T_INT16_ 's'
#define T_UINT16_ 't'
#define T_INT_ 'i'
#define T_UINT_ 'j'
#define T_INT64_ 'x'
#define T_UINT64_ 'y'
#define T_BOOL_ 'b'
#define T_DOUBLE_ 'd'
#define T_FLOAT_ 'f'
#define T_TM_ 'm'
#define T_TEXT_ 'e'
#define T_STRING_ 'c'
#define T_POINTER_ '*'
#define T_VECTOR_ '_'

#endif
