#ifndef STR_MAP_H
#define STR_MAP_H
#include <unordered_map>
#include <hpp/string_view.hpp>
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
  struct str_hash {
    unsigned int operator()(const std::string& z) const;
  };
  struct str_key_eq {
    bool operator()(const std::string& l, const std::string& r) const;
  };
  struct str_map : std::unordered_multimap<std::string, std::string, str_hash, str_key_eq>{
    _FORCE_INLINE std::string& operator[](std::string&& k) {
      auto _ = this->find(k); if (_ != this->end()) return _->second;
      return this->emplace(std::move(k), std::string{"", 0})->second;
    }
  };

  struct sv_hash {
    unsigned int operator()(const std::string_view& z) const;
  };
  struct sv_key_eq {
    bool operator()(const std::string_view& l, const std::string_view& r) const;
  };
  using sv_map = std::unordered_multimap<std::string_view, std::string_view, sv_hash, sv_key_eq>;
  static const sv_key_eq SV_KEY_EQ;
}
#endif // STR_MAP_H
