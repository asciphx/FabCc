#ifndef CHAR_MAP_H
#define CHAR_MAP_H
#include <unordered_map>
#include <string>
// from https://github.com/ipkn/crow/blob/master/include/crow/ci_map.h
// from https://github.com/boostorg/beast/blob/develop/include/boost/beast/http/impl/field.ipp
namespace cc {
#if defined(_MSC_VER)
#define _INLINE __forceinline
#else
#define _INLINE inline
#endif
  struct char_hash {
	_INLINE size_t operator()(const std::string& z);

  };
  struct char_key_eq {
	_INLINE bool operator()(const std::string& l, const std::string& r);
  };
  using char_map = std::unordered_multimap<std::string, std::string, char_hash, char_key_eq>;
  static const char_key_eq KEY_EQUALS;
#undef _INLINE
}
#endif // CHAR_MAP_H
