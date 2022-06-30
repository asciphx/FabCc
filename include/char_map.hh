#ifndef CHAR_MAP_H
#define CHAR_MAP_H
#include <unordered_map>
#include <string>
// from https://github.com/ipkn/crow/blob/master/include/crow/ci_map.h
// from https://github.com/boostorg/beast/blob/develop/include/boost/beast/http/impl/field.ipp
namespace cc {
  struct char_hash; struct char_key_eq;
  using char_map = std::unordered_multimap<std::string, std::string, char_hash, char_key_eq>;
}
#endif // CHAR_MAP_H
