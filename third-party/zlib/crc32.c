
#ifdef MAKECRCH
# include <stdio.h>
# ifndef DYNAMIC_CRC_TABLE
# define DYNAMIC_CRC_TABLE
# endif 
#endif 

#include "zutil.h" 
 
#ifdef Z_TESTN
# define N Z_TESTN
#else
# define N 5
#endif
#if N < 1 || N > 6
# error N must be in 1..6
#endif

#ifdef Z_TESTW
# if Z_TESTW-1 != -1
# define W Z_TESTW
# endif
#else
# ifdef MAKECRCH
# define W 8 
# else
# if defined(__x86_64__) || defined(__aarch64__)
# define W 8
# else
# define W 4
# endif
# endif
#endif
#ifdef W
# if W == 8 && defined(Z_U8)
 typedef Z_U8 z_word_t;
# elif defined(Z_U4)
# undef W
# define W 4
 typedef Z_U4 z_word_t;
# else
# undef W
# endif
#endif
local z_crc_t multmodp OF((z_crc_t a, z_crc_t b));local z_crc_t x2nmodp OF((z_off64_t n, unsigned k));
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && W == 8
# define ARMCRC32
#endif

#if defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_CRC_TABLE))
local z_word_t byte_swap(word) z_word_t word;{
# if W == 8
 return (word & 0xff00000000000000) >> 56 | (word & 0xff000000000000) >> 40 | (word & 0xff0000000000) >> 24 | (word & 0xff00000000) >> 8 | (word & 0xff000000) << 8 | (word & 0xff0000) << 24 | (word & 0xff00) << 40 | (word & 0xff) << 56;
# else 
 return (word & 0xff000000) >> 24 | (word & 0xff0000) >> 8 | (word & 0xff00) << 8 | (word & 0xff) << 24;
# endif
}
#endif

#define POLY 0xedb88320 

#ifdef DYNAMIC_CRC_TABLE
local z_crc_t FAR crc_table[256];local z_crc_t FAR x2n_table[32];local void make_crc_table OF((void));
#ifdef W
 local z_word_t FAR crc_big_table[256]; local z_crc_t FAR crc_braid_table[W][256]; local z_word_t FAR crc_braid_big_table[W][256]; local void braid OF((z_crc_t [][256], z_word_t [][256], int, int));
#endif
#ifdef MAKECRCH
 local void write_table OF((FILE *, const z_crc_t FAR *, int)); local void write_table32hi OF((FILE *, const z_word_t FAR *, int)); local void write_table64 OF((FILE *, const z_word_t FAR *, int));
#endif 
typedef struct once_s once_t;local void once OF((once_t *, void (*)(void)));
#if defined(__STDC__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)

#include <stdatomic.h>
struct once_s { atomic_flag begun; atomic_int done;};
#define ONCE_INIT {ATOMIC_FLAG_INIT, 0}
local void once(state, init) once_t *state; void (*init)(void);{ if (!atomic_load(&state->done)) { if (atomic_flag_test_and_set(&state->begun)) while (!atomic_load(&state->done)) ; else { init(); atomic_store(&state->done, 1); } }}
#else 
struct once_s { volatile int begun; volatile int done;};
#define ONCE_INIT {0, 0}
local int test_and_set OF((int volatile *));local int test_and_set(flag) int volatile *flag;{ int was; was = *flag; *flag = 1; return was;}local void once(state, init) once_t *state; void (*init)(void);{ if (!state->done) { if (test_and_set(&state->begun)) while (!state->done) ; else { init(); state->done = 1; } }}
#endif
local once_t made = ONCE_INIT;local void make_crc_table(){ unsigned i, j, n; z_crc_t p; for (i = 0; i < 256; i++) { p = i; for (j = 0; j < 8; j++) p = p & 1 ? (p >> 1) ^ POLY : p >> 1; crc_table[i] = p;
#ifdef W
 crc_big_table[i] = byte_swap(p);
#endif
 } p = (z_crc_t)1 << 30;  x2n_table[0] = p; for (n = 1; n < 32; n++) x2n_table[n] = p = multmodp(p, p);
#ifdef W
 braid(crc_braid_table, crc_braid_big_table, N, W);
#endif

#ifdef MAKECRCH
 { 
#if !defined(W) || W != 8
# error Need a 64-bit integer type in order to generate crc32.h.
#endif
 FILE *out; int k, n; z_crc_t ltl[8][256]; z_word_t big[8][256]; out = fopen("crc32.h", "w"); if (out == NULL) return; fprintf(out, "\n" "\n" "local const z_crc_t FAR crc_table[] = {\n" " "); write_table(out, crc_table, 256); fprintf(out, "};\n"); fprintf(out, "\n" "#ifdef W\n" "\n" "#if W == 8\n" "\n" "local const z_word_t FAR crc_big_table[] = {\n" " "); write_table64(out, crc_big_table, 256); fprintf(out, "};\n"); fprintf(out, "\n" "#else \n" "\n" "local const z_word_t FAR crc_big_table[] = {\n" " "); write_table32hi(out, crc_big_table, 256); fprintf(out, "};\n" "\n" "#endif\n"); for (n = 1; n <= 6; n++) { fprintf(out, "\n" "#if N == %d\n", n); braid(ltl, big, n, 8); fprintf(out, "\n" "#if W == 8\n" "\n" "local const z_crc_t FAR crc_braid_table[][256] = {\n"); for (k = 0; k < 8; k++) { fprintf(out, " {"); write_table(out, ltl[k], 256); fprintf(out, "}%s", k < 7 ? ",\n" : ""); } fprintf(out, "};\n" "\n" "local const z_word_t FAR crc_braid_big_table[][256] = {\n"); for (k = 0; k < 8; k++) { fprintf(out, " {"); write_table64(out, big[k], 256); fprintf(out, "}%s", k < 7 ? ",\n" : ""); } fprintf(out, "};\n"); braid(ltl, big, n, 4); fprintf(out, "\n" "#else \n" "\n" "local const z_crc_t FAR crc_braid_table[][256] = {\n"); for (k = 0; k < 4; k++) { fprintf(out, " {"); write_table(out, ltl[k], 256); fprintf(out, "}%s", k < 3 ? ",\n" : ""); } fprintf(out, "};\n" "\n" "local const z_word_t FAR crc_braid_big_table[][256] = {\n"); for (k = 0; k < 4; k++) { fprintf(out, " {"); write_table32hi(out, big[k], 256); fprintf(out, "}%s", k < 3 ? ",\n" : ""); } fprintf(out, "};\n" "\n" "#endif\n" "\n" "#endif\n"); } fprintf(out, "\n" "#endif\n"); fprintf(out, "\n" "local const z_crc_t FAR x2n_table[] = {\n" " "); write_table(out, x2n_table, 32); fprintf(out, "};\n"); fclose(out); }
#endif 
}
#ifdef MAKECRCH
local void write_table(out, table, k) FILE *out; const z_crc_t FAR *table; int k;{ int n; for (n = 0; n < k; n++) fprintf(out, "%s0x%08lx%s", n == 0 || n % 5 ? "" : " ", (unsigned long)(table[n]), n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));}local void write_table32hi(out, table, k)FILE *out;const z_word_t FAR *table;int k;{ int n; for (n = 0; n < k; n++) fprintf(out, "%s0x%08lx%s", n == 0 || n % 5 ? "" : " ", (unsigned long)(table[n] >> 32), n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));}local void write_table64(out, table, k) FILE *out; const z_word_t FAR *table; int k;{ int n; for (n = 0; n < k; n++) fprintf(out, "%s0x%016llx%s", n == 0 || n % 3 ? "" : " ", (unsigned long long)(table[n]), n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));}int main(){ make_crc_table(); return 0;}
#endif 

#ifdef W
local void braid(ltl, big, n, w) z_crc_t ltl[][256]; z_word_t big[][256]; int n; int w;{ int k; z_crc_t i, p, q; for (k = 0; k < w; k++) { p = x2nmodp((n * w + 3 - k) << 3, 0); ltl[k][0] = 0; big[w - 1 - k][0] = 0; for (i = 1; i < 256; i++) { ltl[k][i] = q = multmodp(i << 24, p); big[w - 1 - k][i] = byte_swap(q); } }}
#endif

#else 

#include "crc32.h"
#endif 
local z_crc_t multmodp(a, b) z_crc_t a; z_crc_t b;{ z_crc_t m, p; m = (z_crc_t)1 << 31; p = 0; for (;;) { if (a & m) { p ^= b; if ((a & (m - 1)) == 0) break; } m >>= 1; b = b & 1 ? (b >> 1) ^ POLY : b >> 1; } return p;}local z_crc_t x2nmodp(n, k) z_off64_t n; unsigned k;{ z_crc_t p; p = (z_crc_t)1 << 31;  while (n) { if (n & 1) p = multmodp(x2n_table[k & 31], p); n >>= 1; k++; } return p;}const z_crc_t FAR * ZEXPORT get_crc_table(){
#ifdef DYNAMIC_CRC_TABLE
 once(&made, make_crc_table);
#endif 
 return (const z_crc_t FAR *)crc_table;}
#ifdef ARMCRC32

#define Z_BATCH 3990 
#define Z_BATCH_ZEROS 0xa10d3d0c 
#define Z_BATCH_MIN 800 
unsigned long ZEXPORT crc32_z(crc, buf, len) unsigned long crc; const unsigned char FAR *buf; z_size_t len;{ z_crc_t val; z_word_t crc1, crc2; const z_word_t *word; z_word_t val0, val1, val2; z_size_t last, last2, i; z_size_t num; if (buf == Z_NULL) return 0;
#ifdef DYNAMIC_CRC_TABLE
 once(&made, make_crc_table);
#endif 
 crc = (~crc) & 0xffffffff; while (len && ((z_size_t)buf & 7) != 0) { len--; val = *buf++; __asm__ volatile("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val)); } word = (z_word_t const *)buf; num = len >> 3; len &= 7; while (num >= 3 * Z_BATCH) { crc1 = 0; crc2 = 0; for (i = 0; i < Z_BATCH; i++) { val0 = word[i]; val1 = word[i + Z_BATCH]; val2 = word[i + 2 * Z_BATCH]; __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0)); __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1)); __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2)); } word += 3 * Z_BATCH; num -= 3 * Z_BATCH; crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc1; crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc2; }  last = num / 3; if (last >= Z_BATCH_MIN) { last2 = last << 1; crc1 = 0; crc2 = 0; for (i = 0; i < last; i++) { val0 = word[i]; val1 = word[i + last]; val2 = word[i + last2]; __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0)); __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1)); __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2)); } word += 3 * last; num -= 3 * last; val = x2nmodp(last, 6); crc = multmodp(val, crc) ^ crc1; crc = multmodp(val, crc) ^ crc2; } for (i = 0; i < num; i++) { val0 = word[i]; __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0)); } word += num; buf = (const unsigned char FAR *)word; while (len) { len--; val = *buf++; __asm__ volatile("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val)); } return crc ^ 0xffffffff;}
#else

#ifdef W
local z_crc_t crc_word(data) z_word_t data;{ int k; for (k = 0; k < W; k++) data = (data >> 8) ^ crc_table[data & 0xff]; return (z_crc_t)data;}local z_word_t crc_word_big(data) z_word_t data;{ int k; for (k = 0; k < W; k++) data = (data << 8) ^ crc_big_table[(data >> ((W - 1) << 3)) & 0xff]; return data;}
#endif
unsigned long ZEXPORT crc32_z(crc, buf, len) unsigned long crc; const unsigned char FAR *buf; z_size_t len;{ if (buf == Z_NULL) return 0;
#ifdef DYNAMIC_CRC_TABLE
 once(&made, make_crc_table);
#endif 
 crc = (~crc) & 0xffffffff;
#ifdef W
 if (len >= N * W + W - 1) { z_size_t blks; z_word_t const *words; unsigned endian; int k; while (len && ((z_size_t)buf & (W - 1)) != 0) { len--; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; } blks = len / (N * W); len -= blks * N * W; words = (z_word_t const *)buf; endian = 1; if (*(unsigned char *)&endian) { z_crc_t crc0; z_word_t word0;
#if N > 1
 z_crc_t crc1; z_word_t word1;
#if N > 2
 z_crc_t crc2; z_word_t word2;
#if N > 3
 z_crc_t crc3; z_word_t word3;
#if N > 4
 z_crc_t crc4; z_word_t word4;
#if N > 5
 z_crc_t crc5; z_word_t word5;
#endif
#endif
#endif
#endif
#endif
 crc0 = crc;
#if N > 1
 crc1 = 0;
#if N > 2
 crc2 = 0;
#if N > 3
 crc3 = 0;
#if N > 4
 crc4 = 0;
#if N > 5
 crc5 = 0;
#endif
#endif
#endif
#endif
#endif
 while (--blks) { word0 = crc0 ^ words[0];
#if N > 1
 word1 = crc1 ^ words[1];
#if N > 2
 word2 = crc2 ^ words[2];
#if N > 3
 word3 = crc3 ^ words[3];
#if N > 4
 word4 = crc4 ^ words[4];
#if N > 5
 word5 = crc5 ^ words[5];
#endif
#endif
#endif
#endif
#endif
 words += N; crc0 = crc_braid_table[0][word0 & 0xff];
#if N > 1
 crc1 = crc_braid_table[0][word1 & 0xff];
#if N > 2
 crc2 = crc_braid_table[0][word2 & 0xff];
#if N > 3
 crc3 = crc_braid_table[0][word3 & 0xff];
#if N > 4
 crc4 = crc_braid_table[0][word4 & 0xff];
#if N > 5
 crc5 = crc_braid_table[0][word5 & 0xff];
#endif
#endif
#endif
#endif
#endif
 for (k = 1; k < W; k++) { crc0 ^= crc_braid_table[k][(word0 >> (k << 3)) & 0xff];
#if N > 1
 crc1 ^= crc_braid_table[k][(word1 >> (k << 3)) & 0xff];
#if N > 2
 crc2 ^= crc_braid_table[k][(word2 >> (k << 3)) & 0xff];
#if N > 3
 crc3 ^= crc_braid_table[k][(word3 >> (k << 3)) & 0xff];
#if N > 4
 crc4 ^= crc_braid_table[k][(word4 >> (k << 3)) & 0xff];
#if N > 5
 crc5 ^= crc_braid_table[k][(word5 >> (k << 3)) & 0xff];
#endif
#endif
#endif
#endif
#endif
 } } crc = crc_word(crc0 ^ words[0]);
#if N > 1
 crc = crc_word(crc1 ^ words[1] ^ crc);
#if N > 2
 crc = crc_word(crc2 ^ words[2] ^ crc);
#if N > 3
 crc = crc_word(crc3 ^ words[3] ^ crc);
#if N > 4
 crc = crc_word(crc4 ^ words[4] ^ crc);
#if N > 5
 crc = crc_word(crc5 ^ words[5] ^ crc);
#endif
#endif
#endif
#endif
#endif
 words += N; } else { z_word_t crc0, word0, comb;
#if N > 1
 z_word_t crc1, word1;
#if N > 2
 z_word_t crc2, word2;
#if N > 3
 z_word_t crc3, word3;
#if N > 4
 z_word_t crc4, word4;
#if N > 5
 z_word_t crc5, word5;
#endif
#endif
#endif
#endif
#endif
 crc0 = byte_swap(crc);
#if N > 1
 crc1 = 0;
#if N > 2
 crc2 = 0;
#if N > 3
 crc3 = 0;
#if N > 4
 crc4 = 0;
#if N > 5
 crc5 = 0;
#endif
#endif
#endif
#endif
#endif
 while (--blks) { word0 = crc0 ^ words[0];
#if N > 1
 word1 = crc1 ^ words[1];
#if N > 2
 word2 = crc2 ^ words[2];
#if N > 3
 word3 = crc3 ^ words[3];
#if N > 4
 word4 = crc4 ^ words[4];
#if N > 5
 word5 = crc5 ^ words[5];
#endif
#endif
#endif
#endif
#endif
 words += N; crc0 = crc_braid_big_table[0][word0 & 0xff];
#if N > 1
 crc1 = crc_braid_big_table[0][word1 & 0xff];
#if N > 2
 crc2 = crc_braid_big_table[0][word2 & 0xff];
#if N > 3
 crc3 = crc_braid_big_table[0][word3 & 0xff];
#if N > 4
 crc4 = crc_braid_big_table[0][word4 & 0xff];
#if N > 5
 crc5 = crc_braid_big_table[0][word5 & 0xff];
#endif
#endif
#endif
#endif
#endif
 for (k = 1; k < W; k++) { crc0 ^= crc_braid_big_table[k][(word0 >> (k << 3)) & 0xff];
#if N > 1
 crc1 ^= crc_braid_big_table[k][(word1 >> (k << 3)) & 0xff];
#if N > 2
 crc2 ^= crc_braid_big_table[k][(word2 >> (k << 3)) & 0xff];
#if N > 3
 crc3 ^= crc_braid_big_table[k][(word3 >> (k << 3)) & 0xff];
#if N > 4
 crc4 ^= crc_braid_big_table[k][(word4 >> (k << 3)) & 0xff];
#if N > 5
 crc5 ^= crc_braid_big_table[k][(word5 >> (k << 3)) & 0xff];
#endif
#endif
#endif
#endif
#endif
 } } comb = crc_word_big(crc0 ^ words[0]);
#if N > 1
 comb = crc_word_big(crc1 ^ words[1] ^ comb);
#if N > 2
 comb = crc_word_big(crc2 ^ words[2] ^ comb);
#if N > 3
 comb = crc_word_big(crc3 ^ words[3] ^ comb);
#if N > 4
 comb = crc_word_big(crc4 ^ words[4] ^ comb);
#if N > 5
 comb = crc_word_big(crc5 ^ words[5] ^ comb);
#endif
#endif
#endif
#endif
#endif
 words += N; crc = byte_swap(comb); } buf = (unsigned char const *)words; }
#endif 
 while (len >= 8) { len -= 8; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; } while (len) { len--; crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff]; } return crc ^ 0xffffffff;}
#endif
unsigned long ZEXPORT crc32(crc, buf, len) unsigned long crc; const unsigned char FAR *buf; uInt len;{ return crc32_z(crc, buf, len);}uLong ZEXPORT crc32_combine64(crc1, crc2, len2) uLong crc1; uLong crc2; z_off64_t len2;{
#ifdef DYNAMIC_CRC_TABLE
 once(&made, make_crc_table);
#endif 
 return multmodp(x2nmodp(len2, 3), crc1) ^ (crc2 & 0xffffffff);}uLong ZEXPORT crc32_combine(crc1, crc2, len2) uLong crc1; uLong crc2; z_off_t len2;{ return crc32_combine64(crc1, crc2, (z_off64_t)len2);}uLong ZEXPORT crc32_combine_gen64(len2) z_off64_t len2;{
#ifdef DYNAMIC_CRC_TABLE
 once(&made, make_crc_table);
#endif 
 return x2nmodp(len2, 3);}uLong ZEXPORT crc32_combine_gen(len2) z_off_t len2;{ return crc32_combine_gen64((z_off64_t)len2);}uLong ZEXPORT crc32_combine_op(crc1, crc2, op) uLong crc1; uLong crc2; uLong op;{ return multmodp(op, crc1) ^ (crc2 & 0xffffffff);}