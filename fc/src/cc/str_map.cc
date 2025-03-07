#include <hh/str_map.hh>
#include <immintrin.h>
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
  // Precomputed case conversion table (256 bytes)
  static const unsigned char case_table[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // A-O -> a-o
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // P-Z -> p-z
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // a-o
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // p-z
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
  };
  size_t str_hash::operator()(const std::string& z) const {
    size_t n = z.length(); size_t r = n - 1;
    unsigned char const* p = reinterpret_cast<unsigned char const*>(z.c_str());
    if (n >= 16) {
      const __m128i mul_factor = _mm_set1_epi32(5);
      const __m128i mask_case = _mm_set1_epi8(~0x20);
      const __m128i zero = _mm_setzero_si128();
      __m128i hash_vec = _mm_set1_epi32(static_cast<int>(r));
      const unsigned char* end = p + (n & ~15);
      while (p + 32 <= end) {
        __m128i chunk1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
        chunk1 = _mm_and_si128(chunk1, mask_case);
        __m128i lo1_64 = _mm_unpacklo_epi32(chunk1, zero);
        __m128i hi1_64 = _mm_unpackhi_epi32(chunk1, zero);
        __m128i hash_lo1 = _mm_mul_epu32(hash_vec, mul_factor);
        __m128i hash_hi1 = _mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor);
        hash_vec = _mm_add_epi32(_mm_add_epi32(hash_lo1, lo1_64), _mm_add_epi32(hash_hi1, hi1_64));
        __m128i chunk2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p + 16));
        chunk2 = _mm_and_si128(chunk2, mask_case);
        __m128i lo2_64 = _mm_unpacklo_epi32(chunk2, zero);
        __m128i hi2_64 = _mm_unpackhi_epi32(chunk2, zero);
        __m128i hash_lo2 = _mm_mul_epu32(hash_vec, mul_factor);
        __m128i hash_hi2 = _mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor);
        hash_vec = _mm_add_epi32(_mm_add_epi32(hash_lo2, lo2_64), _mm_add_epi32(hash_hi2, hi2_64));
        p += 32;
      }
      while (p + 16 <= end) {
        __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
        chunk = _mm_and_si128(chunk, mask_case);
        __m128i lo_64 = _mm_unpacklo_epi32(chunk, zero);
        __m128i hi_64 = _mm_unpackhi_epi32(chunk, zero);
        __m128i hash_lo = _mm_mul_epu32(hash_vec, mul_factor);
        __m128i hash_hi = _mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor);
        hash_vec = _mm_add_epi32(_mm_add_epi32(hash_lo, lo_64), _mm_add_epi32(hash_hi, hi_64));
        p += 16;
      }
      r = _mm_cvtsi128_si32(hash_vec); n &= 15;
    }
    while (n >= 8) {
      uint64_t chunk = (static_cast<uint64_t>(case_table[p[0]]) | (static_cast<uint64_t>(case_table[p[1]]) << 8) |
        (static_cast<uint64_t>(case_table[p[2]]) << 16) | (static_cast<uint64_t>(case_table[p[3]]) << 24) |
        (static_cast<uint64_t>(case_table[p[4]]) << 32) | (static_cast<uint64_t>(case_table[p[5]]) << 40) |
        (static_cast<uint64_t>(case_table[p[6]]) << 48) | (static_cast<uint64_t>(case_table[p[7]]) << 56));
      r = r * 5 + chunk; p += 8; n -= 8;
    }
    while (n >= 4) {
      uint32_t chunk = (case_table[p[0]] | (case_table[p[1]] << 8) |
        (case_table[p[2]] << 16) | (case_table[p[3]] << 24));
      r = r * 5 + chunk; p += 4; n -= 4;
    }
    while (n > 0) { r = r * 5 + case_table[*p]; ++p; --n; } return r - z.length();
  }
  bool str_key_eq::operator()(const std::string& l, const std::string& r) const {
    size_t n = l.length(); if (n != r.length()) return false;
    unsigned char const* x = reinterpret_cast<unsigned char const*>(l.c_str());
    unsigned char const* y = reinterpret_cast<unsigned char const*>(r.c_str());
    if (n >= 16) {
      const __m128i mask_case = _mm_set1_epi8(~0x20);
      const unsigned char* end = x + (n & ~15);
      while (x < end) {
        __m128i chunk_x = _mm_loadu_si128(reinterpret_cast<const __m128i*>(x));
        __m128i chunk_y = _mm_loadu_si128(reinterpret_cast<const __m128i*>(y));
        chunk_x = _mm_and_si128(chunk_x, mask_case);
        chunk_y = _mm_and_si128(chunk_y, mask_case);
        __m128i cmp = _mm_cmpeq_epi8(chunk_x, chunk_y);
        if (_mm_movemask_epi8(cmp) != 0xFFFF) return false;
        x += 16; y += 16;
      }
      n &= 15;
    }
    for (; n >= 8; x += 8, y += 8, n -= 8) {
      if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24) |
        (static_cast<size_t>(x[4]) << 32) | (static_cast<size_t>(x[5]) << 40) |
        (static_cast<size_t>(x[6]) << 48) | (static_cast<size_t>(x[7]) << 56)) ^
        (y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24) |
          (static_cast<size_t>(y[4]) << 32) | (static_cast<size_t>(y[5]) << 40) |
          (static_cast<size_t>(y[6]) << 48) | (static_cast<size_t>(y[7]) << 56))) & _m) return false;
    }
    for (; n >= 4; x += 4, y += 4, n -= 4) {
      if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24)) ^
        (y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24))) & _um) return false;
    }
    for (; n; ++x, ++y, --n) if ((*x ^ *y) & 0xDF) return false; return true;
  }
  size_t sv_hash::operator()(const std::string_view& z) const {
    size_t n = z.size(); size_t r = 0;
    unsigned char const* p = reinterpret_cast<unsigned char const*>(z.data());
    if (n >= 16) {
      const __m128i mul_factor = _mm_set1_epi32(5);
      const __m128i mask_case = _mm_set1_epi8(~0x20);
      const __m128i zero = _mm_setzero_si128();
      __m128i hash_vec = _mm_set1_epi32(static_cast<int>(r));
      const unsigned char* end = p + (n & ~15);
      while (p + 32 <= end) {
        __m128i chunk1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
        chunk1 = _mm_and_si128(chunk1, mask_case);
        __m128i lo1_64 = _mm_unpacklo_epi32(chunk1, zero);
        __m128i hi1_64 = _mm_unpackhi_epi32(chunk1, zero);
        __m128i hash_lo1 = _mm_mul_epu32(hash_vec, mul_factor);
        __m128i hash_hi1 = _mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor);
        hash_vec = _mm_add_epi32(_mm_add_epi32(hash_lo1, lo1_64), _mm_add_epi32(hash_hi1, hi1_64));
        __m128i chunk2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p + 16));
        chunk2 = _mm_and_si128(chunk2, mask_case);
        __m128i lo2_64 = _mm_unpacklo_epi32(chunk2, zero);
        __m128i hi2_64 = _mm_unpackhi_epi32(chunk2, zero);
        __m128i hash_lo2 = _mm_mul_epu32(hash_vec, mul_factor);
        __m128i hash_hi2 = _mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor);
        hash_vec = _mm_add_epi32(_mm_add_epi32(hash_lo2, lo2_64), _mm_add_epi32(hash_hi2, hi2_64));
        p += 32;
      }
      while (p + 16 <= end) {
        __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
        chunk = _mm_and_si128(chunk, mask_case);
        __m128i lo_64 = _mm_unpacklo_epi32(chunk, zero);
        __m128i hi_64 = _mm_unpackhi_epi32(chunk, zero);
        __m128i hash_lo = _mm_mul_epu32(hash_vec, mul_factor);
        __m128i hash_hi = _mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor);
        hash_vec = _mm_add_epi32(_mm_add_epi32(hash_lo, lo_64), _mm_add_epi32(hash_hi, hi_64));
        p += 16;
      }
      r = _mm_cvtsi128_si32(hash_vec); n &= 15;
    }
    while (n >= 8) {
      uint64_t chunk = (static_cast<uint64_t>(case_table[p[0]]) | (static_cast<uint64_t>(case_table[p[1]]) << 8) |
        (static_cast<uint64_t>(case_table[p[2]]) << 16) | (static_cast<uint64_t>(case_table[p[3]]) << 24) |
        (static_cast<uint64_t>(case_table[p[4]]) << 32) | (static_cast<uint64_t>(case_table[p[5]]) << 40) |
        (static_cast<uint64_t>(case_table[p[6]]) << 48) | (static_cast<uint64_t>(case_table[p[7]]) << 56));
      r = r * 5 + chunk; p += 8; n -= 8;
    }
    while (n >= 4) {
      uint32_t chunk = (case_table[p[0]] | (case_table[p[1]] << 8) |
        (case_table[p[2]] << 16) | (case_table[p[3]] << 24));
      r = r * 5 + chunk; p += 4; n -= 4;
    }
    while (n > 0) { r = r * 5 + case_table[*p]; ++p; --n; } return r;
  }
  bool sv_key_eq::operator()(const std::string_view& l, const std::string_view& r) const {
    size_t n = l.size(); if (n != r.size()) return false;
    unsigned char const* x = reinterpret_cast<unsigned char const*>(l.data());
    unsigned char const* y = reinterpret_cast<unsigned char const*>(r.data());
    if (n >= 16) {
      const __m128i mask_case = _mm_set1_epi8(~0x20);
      const unsigned char* end = x + (n & ~15);
      while (x < end) {
        __m128i chunk_x = _mm_loadu_si128(reinterpret_cast<const __m128i*>(x));
        __m128i chunk_y = _mm_loadu_si128(reinterpret_cast<const __m128i*>(y));
        chunk_x = _mm_and_si128(chunk_x, mask_case);
        chunk_y = _mm_and_si128(chunk_y, mask_case);
        __m128i cmp = _mm_cmpeq_epi8(chunk_x, chunk_y);
        if (_mm_movemask_epi8(cmp) != 0xFFFF) return false;
        x += 16; y += 16;
      }
      n &= 15;
    }
    for (; n >= 8; x += 8, y += 8, n -= 8) {
      if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24) |
        (static_cast<size_t>(x[4]) << 32) | (static_cast<size_t>(x[5]) << 40) |
        (static_cast<size_t>(x[6]) << 48) | (static_cast<size_t>(x[7]) << 56)) ^
        (y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24) |
          (static_cast<size_t>(y[4]) << 32) | (static_cast<size_t>(y[5]) << 40) |
          (static_cast<size_t>(y[6]) << 48) | (static_cast<size_t>(y[7]) << 56))) & _m) return false;
    }
    for (; n >= 4; x += 4, y += 4, n -= 4) {
      if (((x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24)) ^
        (y[0] | (y[1] << 8) | (y[2] << 16) | (y[3] << 24))) & _um) return false;
    }
    for (; n; ++x, ++y, --n) if ((*x ^ *y) & 0xDF) return false; return true;
  }
}