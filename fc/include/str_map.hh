#ifndef STR_MAP_H
#define STR_MAP_H
#include <unordered_map>
#include <hpp/string_view.hpp>
namespace fc {
  struct str_hash {
	size_t operator()(const std::string_view& z) const;
  };
  struct str_key_eq {
	bool operator()(const std::string_view& l, const std::string_view& r) const;
  };
  using str_map = std::unordered_multimap<std::string_view, std::string_view, str_hash, str_key_eq>;
  static const str_key_eq STR_KEY_EQ;
}
#endif // STR_MAP_H
