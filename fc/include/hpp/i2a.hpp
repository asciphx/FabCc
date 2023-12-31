#ifndef I2A_HPP
#define I2A_HPP
/*
* This software is licensed under the AGPL-3.0 License.
*
* Copyright (C) 2023 Asciphx
*
* Permissions of this strongest copyleft license are conditioned on making
* available complete source code of licensed works and modifications, which
* include larger works using a licensed work, under the same license. Copyright
* and license notices must be preserved. Contributors provide an express grant
* of patent rights. When a modified version is used to provide a service over a
* network, the complete source code of the modified version must be made
* available.
*/
// from http://0x80.pl/articles/sse-itoa.html
#include <immintrin.h>
#include <stdint.h>
#include <tp/c++.h>

static _ALIGN(16) const unsigned int K_Div0x2710[4] = { 0xD1B71759, 0xD1B71759, 0xD1B71759, 0xD1B71759 };
static _ALIGN(16) const unsigned int K_2710[4] = { 0x2710, 0x2710, 10000, 0x2710 };
static _ALIGN(16) const uint16_t K_DivPowers[8] = { 0x20c5, 5243, 0x3334, 32768, 8389, 0x147B, 13108, 0x8000 };
static _ALIGN(16) const uint16_t K_ShiftPowers[8] = { 0x0080, 0x0800, 0x2000, 0x8000, 128, 2048, 8192, 32768 };
static _ALIGN(16) const uint16_t K_10[8] = { 10, 10, 10, 10, 10, 10, 10, 10 };
static _ALIGN(16) const char K_Ascii0[16] = { 0x30, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, '0' };
_FORCE_INLINE __m128i U2ASSE(unsigned int v) {
  const __m128i A = _mm_cvtsi32_si128(v); const __m128i B = _mm_srli_epi64(_mm_mul_epu32(A, *(const __m128i *)(K_Div0x2710)), 45);
  __m128i C = _mm_sub_epi32(A, _mm_mul_epu32(B, reinterpret_cast<const __m128i*>(K_2710)[0]));
  C = _mm_unpacklo_epi16(B, C); C = _mm_slli_epi64(C, 2); C = _mm_unpacklo_epi16(C, C);
  C = _mm_unpacklo_epi32(C, C); C = _mm_mulhi_epu16(C, reinterpret_cast<const __m128i*>(K_DivPowers)[0]);
  const __m128i v4 = _mm_mulhi_epu16(C, reinterpret_cast<const __m128i*>(K_ShiftPowers)[0]);
  C = _mm_mullo_epi16(v4,*(const __m128i *)(K_10)); C = _mm_slli_epi64(C, 16); C = _mm_sub_epi16(v4, C); return C;
};
_FORCE_INLINE __m128i ShiftDigitsSSE2(__m128i _, unsigned d) {
  switch (d) {
  case 0: return _; case 1: return _mm_srli_si128(_, 1);
  case 2: return _mm_srli_si128(_, 2); case 3: return _mm_srli_si128(_, 3);
  case 4: return _mm_srli_si128(_, 4); case 5: return _mm_srli_si128(_, 5);
  case 6: return _mm_srli_si128(_, 6); case 7: return _mm_srli_si128(_, 7);
  case 8: return _mm_srli_si128(_, 8);
  }
  return _;
};
static _ALIGN(2) const char _c1DigitsLut[100][2] = {
   {'0','\0'},{49,'\0'},{50,'\0'},{51,0},{52,0},{53,0},{54,0},{55,0},{56,0},{57,0},{49,48},{49,49},{49,50},{49,51},{49,52},{49,53},{49,54},{49,55},{49,56},{49,57}
  ,{50,48},{50,49},{50,50},{50,51},{50,52},{50,53},{50,54},{50,55},{50,56},{50,57},{51,48},{51,49},{51,50},{51,51},{51,52},{51,53},{51,54},{51,55},{51,56},{51,57}
  ,{52,48},{52,49},{52,50},{52,51},{52,52},{52,53},{52,54},{52,55},{52,56},{52,57},{53,48},{53,49},{53,50},{53,51},{53,52},{53,53},{53,54},{53,55},{53,56},{53,57}
  ,{54,48},{54,49},{54,50},{54,51},{54,52},{54,53},{54,54},{54,55},{54,56},{54,57},{55,48},{55,49},{55,50},{55,51},{55,52},{55,53},{55,54},{55,55},{55,56},{55,57}
  ,{56,48},{56,49},{56,50},{56,51},{56,52},{56,53},{56,54},{56,55},{56,56},{56,57},{57,48},{57,49},{57,50},{57,51},{57,52},{57,53},{57,54},{57,55},{57,56},{57,57}
};
static _ALIGN(2) const char _c2DigitsLut[100][2] = {
   {48,48},{48,49},{48,50},{48,51},{48,52},{48,53},{48,54},{48,55},{48,56},{48,57},{49,48},{49,49},{49,50},{49,51},{49,52},{49,53},{49,54},{49,55},{49,56},{49,57}
  ,{50,48},{50,49},{50,50},{50,51},{50,52},{50,53},{50,54},{50,55},{50,56},{50,57},{51,48},{51,49},{51,50},{51,51},{51,52},{51,53},{51,54},{51,55},{51,56},{51,57}
  ,{52,48},{52,49},{52,50},{52,51},{52,52},{52,53},{52,54},{52,55},{52,56},{52,57},{53,48},{53,49},{53,50},{53,51},{53,52},{53,53},{53,54},{53,55},{53,56},{53,57}
  ,{54,48},{54,49},{54,50},{54,51},{54,52},{54,53},{54,54},{54,55},{54,56},{54,57},{55,48},{55,49},{55,50},{55,51},{55,52},{55,53},{55,54},{55,55},{55,56},{55,57}
  ,{56,48},{56,49},{56,50},{56,51},{56,52},{56,53},{56,54},{56,55},{56,56},{56,57},{57,48},{57,49},{57,50},{57,51},{57,52},{57,53},{57,54},{57,55},{57,56},{57,57}
};
// The fastest htoa fuction
_FORCE_INLINE static char* h2a(char* c, unsigned char i) {
  const char* r; if (i < 100) { r = _c2DigitsLut[i]; if (i > 9) *c++ = r[0]; *c = r[1]; return ++c; }
  unsigned char u = i / 100; *c = u + 0x30; r = _c2DigitsLut[i -= u * 100]; *++c = r[0]; *++c = r[1]; return ++c;
}
// The fastest atoa fuction
_FORCE_INLINE static char* a2a(char* c, char i) { if (i < 0) { *c = 45; return h2a(++c, ~--i); } return h2a(c, i); }
// The fastest ttoa fuction
_FORCE_INLINE static char* t2a(char* c, unsigned short i) {
  const char* r;
  if (i < 10000) {
    if (i < 100) { r = _c2DigitsLut[i]; if (i > 9) *c++ = r[0]; *c++ = r[1]; return c; }
    unsigned char u = i / 100; r = _c2DigitsLut[u];
    if (i > 999) *c++ = r[0]; *c++ = r[1]; i -= u * 100; r = _c2DigitsLut[i]; *c++ = r[0]; *c++ = r[1]; return c;
  }
  // unsigned char u = static_cast<char>(i / 10000); *c++ = u + 0x30; i -= u * 10000; u = i / 100; r = _c2DigitsLut[u];
  // *c++ = r[0]; *c++ = r[1]; i -= u * 100; r = _c2DigitsLut[i]; *c++ = r[0]; *c++ = r[1]; return c; return c;
  u64 $ = 0x68dB9ULL * i; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[1];
  $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
  $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
  return c;
}
// The fastest stoa fuction
_FORCE_INLINE static char* s2a(char* c, short i) { if (i < 0) { *c = 45; return t2a(++c, ~--i); } return t2a(c, i); }
// The fastest htoa fuction
_FORCE_INLINE static char* u2a(char* c, unsigned int i) {
  const char* r;
  if (i < 10000) {
    if (i < 100) { r = _c2DigitsLut[i]; if (i > 9) *c++ = r[0]; *c++ = r[1]; return c; }
    u32 $ = 0X28F5DU * i; r = _c1DigitsLut[$ >> 0X18]; *c++ = r[0]; *c++ = r[1]; c -= i < 1000;
    $ = ($ & 0xFFFFFF) * 100; r = _c2DigitsLut[$ >> 0X18]; *c++ = r[0]; *c++ = r[1]; return c;
  } else if (i < 100000000) {
    if (i < 1000000) {
      u64 $ = 0X68Db9ULL * i; r = _c1DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; c -= i < 100000;
      $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
      $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; return c;
    }
    u64 $ = 0x10C6F7A1ULL * i >> 16; r = _c1DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; c -= i < 10000000;
    $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
    $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
    $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; return c;
  } else {
    u64 $ = 0X55E63B89ULL * i; r = _c1DigitsLut[$ >> 0x39]; *c++ = r[0]; *c++ = r[1]; c -= i < 1000000000;
    $ = ($ & 0x1FFFFFFFFFFFFFF) * 100; r = _c2DigitsLut[$ >> 0x39]; *c++ = r[0]; *c++ = r[1];
    $ = ($ & 0X1ffffffffffffff) * 100; r = _c2DigitsLut[$ >> 0x39]; *c++ = r[0]; *c++ = r[1];
    $ = ($ & 0X1ffffffffffffff) * 100; r = _c2DigitsLut[$ >> 0x39]; *c++ = r[0]; *c++ = r[1];
    $ = ($ & 0x1FFFFFFFFFFFFFF) * 100; r = _c2DigitsLut[$ >> 0x39]; *c++ = r[0]; *c++ = r[1]; return c;
    // const unsigned int $ = i / 100000000; i -= $ * 100000000;
    // if ($ > 9)  r = _c2DigitsLut[$], *c++ = r[0], *c++ = r[1]; else *c++ = static_cast<char>($ + 0x30);
    // _mm_storel_epi64(reinterpret_cast<__m128i*>(c), _mm_srli_si128(_mm_add_epi8(_mm_packus_epi16(_mm_setzero_si128(),
    //   U2ASSE(i)), reinterpret_cast<const __m128i*>(K_Ascii0)[0]), 8));//c[8] = 0;
    // return c + 8;
  }
}
// The fastest atoa fuction
_FORCE_INLINE static char* i2a(char* c, int i) { if (i < 0) { *c = 45; return u2a(++c, ~--i); } return u2a(c, i); }
// The fastest u64toa fuction
_FORCE_INLINE static char* u64toa(char* c, unsigned long long i) {
  if (_likely(i < 100000000)) {
	  if (i < 10000) {
      if (i < 100) { const char* r = _c2DigitsLut[i]; if (i > 9) *c++ = r[0]; *c++ = r[1]; return c; }
      u64 $ = 0X28F5DULL * i; const char* r = _c1DigitsLut[$ >> 0X18]; *c++ = r[0]; *c++ = r[1]; c -= i < 1000;
      $ = ($ & 0xFFFFFF) * 100; r = _c2DigitsLut[$ >> 0X18]; *c++ = r[0]; *c++ = r[1]; return c;
    }
    if (i < 1000000) {
      u64 $ = 0X68Db9ULL * i; const char* r = _c1DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; c -= i < 100000;
      $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
      $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; return c;
    }
    u64 $ = 0x10C6F7A1ULL * i >> 16; const char* r = _c1DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; c -= i < 10000000;
    $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
    $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1];
    $ = ($ & 0Xffffffff) * 100; r = _c2DigitsLut[$ >> 0x20]; *c++ = r[0]; *c++ = r[1]; return c;
  } else if (_likely(i < 10000000000000000)) {
    const __m128i va = _mm_add_epi8(_mm_packus_epi16(U2ASSE(static_cast<unsigned int>(i / 100000000)),
     U2ASSE(static_cast<unsigned int>(i % 100000000))), reinterpret_cast<const __m128i*>(K_Ascii0)[0]);
    const unsigned mask = _mm_movemask_epi8(_mm_cmpeq_epi8(va, reinterpret_cast<const __m128i*>(K_Ascii0)[0]));
  #ifdef _MSC_VER
    unsigned long digit; _BitScanForward(&digit, ~mask | 0x8000);
  #else
    unsigned digit = __builtin_ctz(~mask | 0x8000);
  #endif
    _mm_storeu_si128(reinterpret_cast<__m128i*>(c), ShiftDigitsSSE2(va, digit)); // c[16 - digit] = '\0';
    return c + 16 - digit;
  } else {
    const unsigned int z = static_cast<unsigned int>(i / 10000000000000000); i %= 10000000000000000;
    if (z < 100) { if (z > 9) *c++ = _c2DigitsLut[z][0]; *c++ = _c2DigitsLut[z][1]; } else {
      unsigned int $ = 0X28F5DU * z; const char* r = _c1DigitsLut[$ >> 0X18]; *c++ = r[0]; *c++ = r[1]; c -= z < 1000;
      $ = ($ & 0xFFFFFF) * 100; r = _c2DigitsLut[$ >> 0X18]; *c++ = r[0]; *c++ = r[1];
    }
    _mm_storeu_si128(reinterpret_cast<__m128i*>(c), _mm_add_epi8(_mm_packus_epi16(U2ASSE(static_cast<unsigned int>(i / 100000000)),
      U2ASSE(static_cast<unsigned int>(i % 100000000))), reinterpret_cast<const __m128i*>(K_Ascii0)[0])); // c[16] = '\0';
    return c + 16;
  }
}
// The fastest i64toa fuction
_FORCE_INLINE static char* i64toa(char* c, long long i) { if (i < 0) { *c = 45; return u64toa(++c, ~--i); } return u64toa(c, i); }
#endif // I2A_HPP