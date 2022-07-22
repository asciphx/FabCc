#ifndef STR_MAP_H
#define STR_MAP_H
#include <unordered_map>
#include <string>
// from https://github.com/ipkn/crow/blob/master/include/crow/ci_map.h
// from https://github.com/boostorg/beast/blob/develop/include/boost/beast/http/impl/field.ipp
namespace fc {
  struct str_hash {
	size_t operator()(const std::string& z) const;
  };
  struct str_key_eq {
	bool operator()(const std::string& l, const std::string& r) const;
  };
  using str_map = std::unordered_multimap<std::string, std::string, str_hash, str_key_eq>;
  static const str_key_eq KEY_EQUALS;
}
#endif // STR_MAP_H
