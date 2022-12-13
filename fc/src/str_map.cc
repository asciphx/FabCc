#include <str_map.hh>
namespace fc {
  size_t str_hash::operator()(const std::string_view& z) const {
	size_t r = 0, n = z.size();
	unsigned char const* p = reinterpret_cast<unsigned char const*>(z.data());
	while (n >= 4) {
	  r = (r * 5 + ((p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)) & ~0x20202020));
	  p += 4; n -= 4;
	}
	while (n > 0) { r = r * 5 + (*p | 0x20); ++p; --n; } return r;
  }
  bool str_key_eq::operator()(const std::string_view& l, const std::string_view& r) const {
	size_t n = l.size(); if (n != r.size()) return false;
	unsigned char const* x = reinterpret_cast<unsigned char const*>(l.data());
	unsigned char const* y = reinterpret_cast<unsigned char const*>(r.data());
	size_t z = sizeof(size_t), m = 0xDFDFDFDFDFDFDFDF & ~std::uint32_t{ 0 };
	for (; n >= z; x += z, y += z, n -= z) {
	  if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24)) ^
		(y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24))) & m) return false;
	}
	for (; n; ++x, ++y, --n) if ((*x ^ *y) & 0xDF) return false; return true;
  }
}
