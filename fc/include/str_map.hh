#ifndef STR_MAP_H
#define STR_MAP_H
#include <unordered_map>
#include <buf.hh>
// from https://github.com/ipkn/crow/blob/master/include/crow/ci_map.h
// from https://github.com/boostorg/beast/blob/develop/include/boost/beast/http/impl/field.ipp
namespace fc {
  struct str_hash {
	unsigned int operator()(const fc::Buf& z) const;
  };
  struct str_key_eq {
	bool operator()(const fc::Buf& l, const fc::Buf& r) const;
  };
  using str_map = std::unordered_multimap<fc::Buf, fc::Buf, str_hash, str_key_eq>;
  static const str_key_eq STR_KEY_EQ;
}
#endif // STR_MAP_H
