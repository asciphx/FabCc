#include <str_map.hh>
// from https://github.com/ipkn/crow/blob/master/include/crow/ci_map.h
// from https://github.com/boostorg/beast/blob/develop/include/boost/beast/http/impl/field.ipp
namespace fc {
  size_t str_hash::operator()(const std::string& z) const {
	size_t r = 0, n = z.size();
	unsigned char const* p = reinterpret_cast<unsigned char const*>(z.data());
	while (n >= 4) {
	  r = (r * 5 + ((p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)) & ~0x20202020));
	  p += 4; n -= 4;
	}
	while (n > 0) { r = r * 5 + (*p | 0x20); ++p; --n; } return r;
  }
  bool str_key_eq::operator()(const std::string& l, const std::string& r) const {
	unsigned long long n = l.length(); if (n != r.length()) return false;
	unsigned char const* x = reinterpret_cast<unsigned char const*>(l.data());
	unsigned char const* y = reinterpret_cast<unsigned char const*>(r.data());
	unsigned int z = sizeof(unsigned int), m = 0xDFDFDFDF & ~std::uint32_t{ 0 };
	for (; n >= z; x += z, y += z, n -= z) {
	  if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24)) ^
		(y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24))) & m) return false;
	}
	for (; n; ++x, ++y, --n) if ((*x ^ *y) & 0xDF) return false; return true;
  }
}
