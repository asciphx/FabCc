#include <hh/str_map.hh>
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
  static constexpr unsigned int m = 0xDFDFDFDFDFDFDFDF & ~uint32_t{ 0 };
  size_t str_hash::operator()(const std::string& z) const {
    size_t n = z.length(); size_t r = n - 1;
    unsigned char const* p = reinterpret_cast<unsigned char const*>(z.c_str());
    while (n >= 4) {
      r = (r * 5 + ((p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)) & ~0x20202020));
      p += 4; n -= 4;
    }
    while (n > 0) { r = r * 5 + (*p | 0x20); ++p; --n; } return r - z.size();
  }
  bool str_key_eq::operator()(const std::string& l, const std::string& r) const {
    size_t n = l.length(); if (n != r.length()) return false;
    unsigned char const* x = reinterpret_cast<unsigned char const*>(l.c_str());
    unsigned char const* y = reinterpret_cast<unsigned char const*>(r.c_str());
    for (; n >= 4; x += 4, y += 4, n -= 4) {
      if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24)) ^
        (y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24))) & m) return false;
    }
    for (; n; ++x, ++y, --n) if ((*x ^ *y) & 0xDF) return false; return true;
  }
  unsigned int sv_hash::operator()(const std::string_view& z) const {
    unsigned int r = 0, n = static_cast<unsigned int>(z.size());
    unsigned char const* p = reinterpret_cast<unsigned char const*>(z.data());
    while (n >= 4) {
      r = (r * 5 + ((p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)) & ~0x20202020));
      p += 4; n -= 4;
    }
    while (n > 0) { r = r * 5 + (*p | 0x20); ++p; --n; } return r;
  }
  bool sv_key_eq::operator()(const std::string_view& l, const std::string_view& r) const {
    unsigned int n = static_cast<unsigned int>(l.size()); if (n != r.size()) return false;
    unsigned char const* x = reinterpret_cast<unsigned char const*>(l.data());
    unsigned char const* y = reinterpret_cast<unsigned char const*>(r.data());
    for (; n >= 4; x += 4, y += 4, n -= 4) {
      if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24)) ^
        (y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24))) & m) return false;
    }
    for (; n; ++x, ++y, --n) if ((*x ^ *y) & 0xDF) return false; return true;
  }
}
