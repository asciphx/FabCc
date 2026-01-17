#include "json.hh"
#include <algorithm>
#include <limits.h>
#ifdef _MSC_VER
#include <intrin.h>
static _FORCE_INLINE int __builtin_ctzZ(unsigned long _) {
  unsigned long _index; _BitScanForward(&_index, _); return static_cast<int>(_index);
}
#else
#define __builtin_ctzZ __builtin_ctz
#endif
namespace json {
  class Parser;
  namespace xx {
    static Json _null;
    static const char s2e_table[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0,'"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '/', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0, 0,
      0, '\b', 0, 0, 0, '\f', 0, 0, 0, 0, 0, 0, 0, '\n', 0, 0, 0, '\r', 0, '\t', 'u' , 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static const char e2s_table[] = {
      0, 0, 0, 0, 0, 0, 0, 0,'b', 't', 'n', 0, 'f', 'r', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, '"' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static const char tb_table[] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,0,1,2,3,4,5,6,7,8,9,16,16,16,16,16,16,16,10,11,12,13,14,15,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,10,11,12,13,14,15,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16 };
    class Alloc {
      Array _a[4], _stack, _ustack; friend Parser; std::string _fs;
    public:
      static const u32 N = 8192; Alloc(): _stack(), _ustack(0x20), _fs(0x400, '\0') {}
      _FORCE_INLINE void* alloc() { if (_a[0].empty()) { return ::malloc(16); } return _a[0].pop_back(); }
      _FORCE_INLINE void free(void* p) { _a[0].size() < ((N - Array::R) << 3) ? _a[0].push_back(p) : ::free(p); }
      _FORCE_INLINE void* alloc(u32 n) {
        void* p; const u32 x = (n - 1) >> 4;
        switch (x) {
        case 0:if (_a[0].empty()) { p = ::malloc(16); break; } p = _a[0].pop_back(); break;
        case 1:if (_a[1].empty()) { p = ::malloc(32); break; } p = _a[1].pop_back(); break;
        case 2:case 3:if (_a[2].empty()) { p = ::malloc(64); break; } p = _a[2].pop_back(); break;
        case 4:case 5:case 6:case 7:if (_a[3].empty()) { p = ::malloc(128); break; } p = _a[3].pop_back(); break;
        default: p = ::malloc(n);
        }
        return p;
      }
      _FORCE_INLINE void free(void* p, u32 n) {
        const u32 x = (n - 1) >> 4;
        switch (x) {
        case 0:
          _a[0].size() < ((N - Array::R) << 3) ? _a[0].push_back(p) : ::free(p);
          break;
        case 1:
          _a[1].size() < ((N - Array::R) << 2) ? _a[1].push_back(p) : ::free(p);
          break;
        case 2:
        case 3:
          _a[2].size() < ((N - Array::R) << 1) ? _a[2].push_back(p) : ::free(p);
          break;
        case 4:
        case 5:
        case 6:
        case 7:
          _a[3].size() < (N - Array::R) ? _a[3].push_back(p) : ::free(p);
          break;
        default:
          ::free(p);
        }
      }
      _FORCE_INLINE std::string& stream() { _fs.clear(); return _fs; }
    };
    static __thread Alloc* _a = 0; _FORCE_INLINE Alloc& jalloc() { if (_a) return *_a; return *(_a = (new (::malloc(sizeof(Alloc))) Alloc())); }
    void* alloc() { return jalloc().alloc(); }
    char* alloc_string(const void* p, size_t n) {
      char* s = static_cast<char*>(jalloc().alloc(static_cast<u32>(n) + 1)); memcpy(s, p, n); s[n] = '\0'; return s;
    }
    _FORCE_INLINE void* alloc_array(void** p, u32 n) {
      _A* h = static_cast<_A*>(::malloc(sizeof(_A) + _PTR_LEN * n)); h->cap = h->size = n; memcpy(h->p, p, _PTR_LEN * n); return h;
    }
  } // xx
  _FORCE_INLINE char* make_key(xx::Alloc& a, const char*& p, size_t n) {
    char* s = static_cast<char*>(a.alloc(static_cast<u32>(n + 1))); memcpy(s, p, n); s[n] = '\0'; return s;
  }
  _FORCE_INLINE char* make_key(xx::Alloc& a, const char*& p) {
    size_t n = strlen(p); char* s = static_cast<char*>(a.alloc(static_cast<u32>(n + 1))); memcpy(s, p, n); s[n] = '\0'; return s;
  }
  _FORCE_INLINE char* make_key(xx::Alloc& a, const std::string_view& p) {
    char* s = static_cast<char*>(a.alloc(static_cast<u32>(p.size() + 1))); memcpy(s, p.data(), p.size()); s[p.size()] = '\0'; return s;
  }
  _FORCE_INLINE Json::_H* make_string(xx::Alloc& a, const void* p, size_t n) { return new(a.alloc()) Json::_H(p, n); }
  _FORCE_INLINE Json::_H* make_object(xx::Alloc& a) { return new(a.alloc()) Json::_H(Json::_obj_t()); }
  _FORCE_INLINE Json::_H* make_array(xx::Alloc& a) { return new(a.alloc()) Json::_H(Json::_arr_t()); }
  // json parser
  //   @b: beginning of the string
  //   @e: end of the string
  // return the current position, or NULL on any error
  class Parser {
    xx::Alloc& _a;
  public:
    Parser(): _a(xx::jalloc()) {} ~Parser() = default; bool parse(const char* b, const char* e, void*& v);
    bool parse_comments(const char* b, const char* e, void*& v); const char* parse_string(const char* b, const char* e, void*& v);
    const char* parse_unicode(const char* b, const char* e, std::string& s); const char* parse_number(const char* b, const char* e, void*& v);
    _FORCE_INLINE const char* parse_key(const char* b, const char* e, void*& k); _FORCE_INLINE const char* parse_false(const char* b, const char* e, void*& v);
    _FORCE_INLINE const char* parse_true(const char* b, const char* e, void*& v); _FORCE_INLINE const char* parse_null(const char* b, const char* e, void*& v);
  };
  _FORCE_INLINE const char* Parser::parse_key(const char* b, const char* e, void*& key) {
    //if (*b++ != '"') return 0; const char* p = static_cast<const char*>(memchr(b, '"', e - b)); if (p) key = make_key(_a, b, p - b); return p;
    if (*b++ != '"') return 0; size_t l = 0; const char* p = static_cast<const char*>(memchr(b, '"', e - b)); $: l += p - b - l;
    if (b[l - 1] == '\\') { ++l; p = static_cast<const char*>(memchr(b + l, '"', e - b - l)); goto $; } if (p)key = make_key(_a, b, l); return p;
  }
  _FORCE_INLINE const char* Parser::parse_false(const char* b, const char* e, void*& v) {
    if (e - b < 5) return 0; if (memcmp(++b, "alse", 4) == 0) { v = new(_a.alloc()) Json::_H(false); return b + 3; } return 0;
  }
  _FORCE_INLINE const char* Parser::parse_true(const char* b, const char* e, void*& v) {
    if (e - b < 4) return 0; if (memcmp(++b, "rue", 3) == 0) { v = new(_a.alloc()) Json::_H(true); return b + 2; } return 0;
  }
  _FORCE_INLINE const char* Parser::parse_null(const char* b, const char* e, void*& v) {
    if (e - b < 4) return 0; if (memcmp(++b, "ull", 3) == 0) { v = 0; return b + 2; } return 0;
  }
  _FORCE_INLINE bool is_white_space(const char c) { return (c == ' ' || c == '\n' || c == '\r' || c == '\t'); }
  //while (b < e && is_white_space(*b));
#define skip_white_space(b, e) \
    if (is_white_space(*b)) { \
        for (++b;;) { \
            if (b + 8 <= e) { \
                if (!is_white_space(b[0])) break; \
                if (!is_white_space(b[1])) { b += 1; break; } \
                if (!is_white_space(b[2])) { b += 2; break; } \
                if (!is_white_space(b[3])) { b += 3; break; } \
                if (!is_white_space(b[4])) { b += 4; break; } \
                if (!is_white_space(b[5])) { b += 5; break; } \
                if (!is_white_space(b[6])) { b += 6; break; } \
                if (!is_white_space(b[7])) { b += 7; break; } \
                b += 8; \
            } else { \
                if (b + 4 <= e) {\
                    if (!is_white_space(b[0])) break; \
                    if (!is_white_space(b[1])) { b += 1; break; } \
                    if (!is_white_space(b[2])) { b += 2; break; } \
                    if (!is_white_space(b[3])) { b += 3; break; } \
                    b += 4; \
                } \
                if (b == e || !is_white_space(b[0])) break; \
                if (b + 1 == e || !is_white_space(b[1])) { b += 1; break; } \
                if (b + 2 == e || !is_white_space(b[2])) { b += 2; break; } \
                b = e; break; \
            } \
        } \
    }
  static _FORCE_INLINE const char* skip_white(const char* b, const char* e) {
    while (e - b >= 16) {
      __m128i chars = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b));
      __m128i spaces = _mm_or_si128(_mm_cmpeq_epi8(chars, _mm_set1_epi8(' ')),
        _mm_cmpeq_epi8(chars, _mm_set1_epi8('\n')));
      spaces = _mm_or_si128(spaces, _mm_cmpeq_epi8(chars, _mm_set1_epi8('\r')));
      spaces = _mm_or_si128(spaces, _mm_cmpeq_epi8(chars, _mm_set1_epi8('\t')));
      int mask = _mm_movemask_epi8(spaces);
      if (mask != 0xFFFF) {
        b += __builtin_ctzZ(~mask);
        return b;
      }
      b += 16;
    }
    skip_white_space(b, e);
    return b;
  };
#if 1
#define skip_white_ b = skip_white(++b, e);
#else
#define skip_white_ ++b; skip_white_space(b, e)
#endif
  // This is a non-recursive implement of json parser.
  // stack: |prev size|prev state|val|....
  bool Parser::parse(const char* b, const char* e, void*& val) {
    //u32 state; u32 size;
    union { u32 state; void* pstate; }; union { u32 size;  void* psize; }; void* key; const char* p; size_t l = 0;
    xx::Array& s = _a._stack; xx::Array& u = _a._ustack; state = size = 0;
    skip_white_space(b, e);
    switch (*b) {
    case '{': goto obj_beg; case '[': goto arr_beg; case '"': b = parse_string(++b, e, val); break;
    case 'f': b = parse_false(b, e, val); break; case 't': b = parse_true(b, e, val); break;
    case 'n': b = parse_null(b, e, val); break; case '\0': return false; default: b = parse_number(b, e, val);
    }
    if (b == 0) goto err; goto end;
  obj_beg:
    u.push_back(psize);  // prev size
    u.push_back(pstate); // prev state
    s.push_back(make_object(_a));
    size = s.size(); // current size
    state = '{';
  obj_val_beg:
    skip_white_; if (*b == '}') goto val_end; if (*b != '"') goto err; ++b;
    p = static_cast<const char*>(memchr(b, '"', e - b)); $: l += p - b - l;
    if (b[l - 1] == '\\') { ++l; p = static_cast<const char*>(memchr(b + l, '"', e - b - l)); goto $; }
    if (p) { key = make_key(_a, b, l); } else goto err; b = p; s.push_back(key);
    skip_white_; if (*b != ':') goto err;
    skip_white_;
    switch (*b) {
    case '"': b = parse_string(++b, e, val); break; case '{': goto obj_beg; case '[': goto arr_beg;
    case 'f': b = parse_false(b, e, val); break; case 't': b = parse_true(b, e, val); break;
    case 'n': b = parse_null(b, e, val); break; case '\0': goto err; default: b = parse_number(b, e, val);
    }
    if (b == 0) goto err; s.push_back(val);
  obj_val_end:
    skip_white_;
    if (*b == ',') goto obj_val_beg;
    if (*b == '}') goto val_end;
    goto err;
  arr_beg:
    u.push_back(psize);  // prev size
    u.push_back(pstate); // prev state
    s.push_back(make_array(_a));
    size = s.size(); // current size
    state = '[';
  arr_val_beg:
    skip_white_;
    if (*b == ']') goto val_end;
    switch (*b) {
    case '"': b = parse_string(++b, e, val); break; case '{': goto obj_beg; case '[': goto arr_beg;
    case 'f': b = parse_false(b, e, val); break; case 't': b = parse_true(b, e, val); break;
    case 'n': b = parse_null(b, e, val); break; case '\0': goto err; default: b = parse_number(b, e, val);
    }
    if (b == 0) goto err; s.push_back(val);
  arr_val_end:
    skip_white_;
    if (*b == ',') goto arr_val_beg; if (*b == ']') goto val_end; goto err;
  val_end:
    if (s.size() > size) {
      void* p = xx::alloc_array(s.data() + size, s.size() - size); s.resize(size); static_cast<Json::_H*>(s.back())->p = p;
    }
    pstate = u.pop_back(); // prev state
    if (state == '{') { psize = u.pop_back(); goto obj_val_end; } if (state == '[') { psize = u.pop_back(); goto arr_val_end; }
    u.pop_back(); val = s.pop_back();
  end://
    assert(s.size() == 0); skip_white_; return b == e;
  err:
    while (s.size() > 0) {
      if (s.size() > size) {
        if (state == '{' && ((s.size() - size) & 1)) s.push_back(0);
        void* p = xx::alloc_array(s.data() + size, s.size() - size);
        s.resize(size); static_cast<Json::_H*>(s.back())->p = p;
      }
      pstate = u.pop_back(); psize = u.pop_back();
      if (state == 0) { val = s.pop_back(); break; }
    }
    assert(s.size() == 0);
    return false;
  }
  const char* Parser::parse_string(const char* b, const char* e, void*& v) {
    const char* p;
    if ((p = static_cast<const char*>(memchr(b, '"', e - b))) == 0) return 0;
    __m128i escape_char = _mm_set1_epi8('\\');
    const char* q = b;
    bool has_escape = false;
    while (q + 16 <= p) {
      __m128i chars = _mm_loadu_si128((const __m128i*)q);
      __m128i escapes = _mm_cmpeq_epi8(chars, escape_char);
      int mask = _mm_movemask_epi8(escapes);
      if (mask) {
        has_escape = true;
        q += __builtin_ctzZ(mask);
        break;
      }
      q += 16;
    }
    while (q < p && *q != '\\') ++q;
    if (!has_escape && q == p) {
      v = make_string(_a, b, p - b);
      return p;
    }
    std::string& s = _a.stream();
    s.clear(); // Ensure the stream starts empty to avoid memory buildup
    s.append(b, q - b);
    while (q < p) {
      if (*q == '\\') {
        if (++q == e) return 0;
        char c = xx::s2e_table[static_cast<u8>(*q)];
        if (c == 0) {
          if (*q != 'u') return 0;
          // Integrated parse_hex and parse_unicode logic
          ++q; // Skip 'u'
          if (q + 4 > e) return 0;
          // Parse 4 hex digits using SSE2
          __m128i chars = _mm_loadu_si128((const __m128i*)q);
          __m128i digits_lower = _mm_sub_epi8(chars, _mm_set1_epi8('0'));
          __m128i digits_upper = _mm_sub_epi8(chars, _mm_set1_epi8('A'));
          __m128i digits_lower_valid = _mm_cmplt_epi8(digits_lower, _mm_set1_epi8(10));
          __m128i digits_upper_valid = _mm_and_si128(
            _mm_cmplt_epi8(digits_upper, _mm_set1_epi8(6)),
            _mm_cmpgt_epi8(digits_upper, _mm_set1_epi8(-1))
          );
          __m128i digits_lower_adjust = _mm_and_si128(digits_lower, digits_lower_valid);
          __m128i digits_upper_adjust = _mm_and_si128(
            _mm_add_epi8(digits_upper, _mm_set1_epi8(10)),
            digits_upper_valid
          );
          __m128i combined = _mm_or_si128(digits_lower_adjust, digits_upper_adjust);
          int mask_lower = _mm_movemask_epi8(digits_lower_valid);
          int mask_upper = _mm_movemask_epi8(digits_upper_valid);
          int mask = mask_lower | mask_upper;
          if ((mask & 0xF) != 0xF) return 0;
          u32 u0 = static_cast<u32>(_mm_extract_epi16(combined, 0) & 0xFF);
          u32 u1 = static_cast<u32>(_mm_extract_epi16(combined, 0) >> 8 & 0xFF);
          u32 u2 = static_cast<u32>(_mm_extract_epi16(combined, 1) & 0xFF);
          u32 u3 = static_cast<u32>(_mm_extract_epi16(combined, 1) >> 8 & 0xFF);
          u32 u = (u0 << 12) | (u1 << 8) | (u2 << 4) | u3;
          q += 3; // Move past the 4 hex digits
          // Integrated parse_unicode logic
          if (0xD7ff < u && u < 0xDC00) {
            if (e - q < 3) return 0;
            if (q[1] != '\\' || q[2] != 'u') return 0;
            q += 3; // Skip "\u"
            if (q + 4 > e) return 0;
            // Parse second set of 4 hex digits
            chars = _mm_loadu_si128((const __m128i*)q);
            digits_lower = _mm_sub_epi8(chars, _mm_set1_epi8('0'));
            digits_upper = _mm_sub_epi8(chars, _mm_set1_epi8('A'));
            digits_lower_valid = _mm_cmplt_epi8(digits_lower, _mm_set1_epi8(10));
            digits_upper_valid = _mm_and_si128(
              _mm_cmplt_epi8(digits_upper, _mm_set1_epi8(6)),
              _mm_cmpgt_epi8(digits_upper, _mm_set1_epi8(-1))
            );
            digits_lower_adjust = _mm_and_si128(digits_lower, digits_lower_valid);
            digits_upper_adjust = _mm_and_si128(
              _mm_add_epi8(digits_upper, _mm_set1_epi8(10)),
              digits_upper_valid
            );
            combined = _mm_or_si128(digits_lower_adjust, digits_upper_adjust);
            mask_lower = _mm_movemask_epi8(digits_lower_valid);
            mask_upper = _mm_movemask_epi8(digits_upper_valid);
            mask = mask_lower | mask_upper;
            if ((mask & 0xF) != 0xF) return 0;
            u0 = static_cast<u32>(_mm_extract_epi16(combined, 0) & 0xFF);
            u1 = static_cast<u32>(_mm_extract_epi16(combined, 0) >> 8 & 0xFF);
            u2 = static_cast<u32>(_mm_extract_epi16(combined, 1) & 0xFF);
            u3 = static_cast<u32>(_mm_extract_epi16(combined, 1) >> 8 & 0xFF);
            u32 v = (u0 << 12) | (u1 << 8) | (u2 << 4) | u3;
            q += 3; // Move past the second set of 4 hex digits
            if (v < 0xDC00 || v > 0xDFFF) return 0;
            u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
          }
          // Encode UTF-8
          if (u < 0x80) {
            s.append({ static_cast<char>(u) });
          } else if (u < 0x800) {
            s.append({ static_cast<char>(0xC0 | (0xFF & (u >> 6))) });
            s.append({ static_cast<char>(0x80 | (0x3F & u)) });
          } else if (u < 0x10000) {
            s.append({ static_cast<char>(0xE0 | (0xFF & (u >> 12))) });
            s.append({ static_cast<char>(0x80 | (0x3F & (u >> 6))) });
            s.append({ static_cast<char>(0x80 | (0x3F & u)) });
          } else {
            //if (u >= 0x110000) return 0; // Invalid Unicode code point
            assert(u < 0x110000);
            s.append({ static_cast<char>(0xF0 | (0xFF & (u >> 18))) });
            s.append({ static_cast<char>(0x80 | (0x3F & (u >> 12))) });
            s.append({ static_cast<char>(0x80 | (0x3F & (u >> 6))) });
            s.append({ static_cast<char>(0x80 | (0x3F & u)) });
          }
        } else {
          s.append({ c });
        }
        b = q + 1;
        p = static_cast<const char*>(memchr(b, '"', e - b));
        if (p == 0) return 0;
        q = b;
        while (q + 16 <= p) {
          __m128i chars = _mm_loadu_si128((const __m128i*)q);
          __m128i escapes = _mm_cmpeq_epi8(chars, escape_char);
          int mask = _mm_movemask_epi8(escapes);
          if (mask) {
            q += __builtin_ctzZ(mask);
            break;
          }
          q += 16;
        }
        while (q < p && *q != '\\') ++q;
        s.append(b, q - b);
      }
    }
    v = make_string(_a, s.data(), s.size());
    return p;
  }
  _FORCE_INLINE const char* parse_hex(const char* b, const char* e, u32& u) {
    u32 u0, u1, u2, u3;
    if (b + 4 <= e) {
      u0 = xx::tb_table[static_cast<u8>(b[0])]; u1 = xx::tb_table[static_cast<u8>(b[1])];
      u2 = xx::tb_table[static_cast<u8>(b[2])]; u3 = xx::tb_table[static_cast<u8>(b[3])];
      if (u0 == 16 || u1 == 16 || u2 == 16 || u3 == 16) return 0;
      u = (u0 << 12) | (u1 << 8) | (u2 << 4) | u3;
      return b + 3;
    }
    return 0;
  }
  // utf8:
  //   0000 - 007F      0xxxxxxx            
  //   0080 - 07FF      110xxxxx  10xxxxxx        
  //   0800 - FFFF      1110xxxx  10xxxxxx  10xxxxxx    
  //  10000 - 10FFFF    11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
  //
  // \uXXXX
  // \uXXXX\uYYYY
  //   D800 <= XXXX <= DBFF
  //   DC00 <= XXXX <= DFFF
  const char* Parser::parse_unicode(const char* b, const char* e, std::string& s) {
    u32 u = 0; b = parse_hex(b, e, u); if (b == 0) return 0;
    if (0xD7ff < u && u < 0xDC00) {
      if (e - b < 3) return 0;
      if (b[1] != '\\' || b[2] != 'u') return 0;
      u32 v = 0;
      b = parse_hex(b + 3, e, v);
      if (b == 0) return 0;
      if (v < 0xDC00 || v > 0xDFFF) return 0;
      u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
    }
    // encode to UTF8
    if (u < 0x80) {
      s.append({ static_cast<char>(u) });
    } else if (u < 0x800) {
      s.append({ static_cast<char>(0xC0 | (0xFF & (u >> 6))) });
      s.append({ static_cast<char>(0x80 | (0x3F & u)) });
    } else if (u < 0x10000) {
      s.append({ static_cast<char>(0xE0 | (0xFF & (u >> 12))) });
      s.append({ static_cast<char>(0x80 | (0x3F & (u >> 6))) });
      s.append({ static_cast<char>(0x80 | (0x3F & u)) });
    } else {
      assert(u < 0x110000);
      s.append({ static_cast<char>(0xF0 | (0xFF & (u >> 18))) });
      s.append({ static_cast<char>(0x80 | (0x3F & (u >> 12))) });
      s.append({ static_cast<char>(0x80 | (0x3F & (u >> 6))) });
      s.append({ static_cast<char>(0x80 | (0x3F & u)) });
    }
    return b;
  }
  _FORCE_INLINE bool str2double(const char*& b, double& d) {
    errno = 0; d = strtod(b, 0); return !(errno == ERANGE && (d == HUGE_VAL || d == -HUGE_VAL));
  }
  const char* Parser::parse_number(const char* b, const char* e, void*& v) {
    const char* p = b; char c;
    if (_unlikely(*b == '-')) {
      ++p; i64 u = '0' - *p; if (u > 0 || u < -9) return 0; while (++p < e) { c = *p; if (0X2f < c && c < 0X3a) {} else break; }
      if (*p == '.' || *p == 'e' || *p == 'E') goto $; c = static_cast<char>(p - ++b); if (c > 19) goto $;
      if (c == 19) { if (memcmp(b, "9223372036854775808", 20) > 0) goto $; }
      while (++b < p) { u = u * 10 + '0' - *b; } v = new(_a.alloc()) Json::_H(u); return --p;
    } else {
      u64 u = *p - '0'; if (u > 9) return 0; while (++p < e) { c = *p; if (0X2f < c && c < 0X3a) {} else break; }
      if (*p == '.' || *p == 'e' || *p == 'E') goto $; c = static_cast<char>(p - b); if (c > 20) goto $;
      if (c == 20) { if (memcmp(b, "18446744073709551615", 20) > 0) goto $; } while (++b < p) { u = u * 10 + *b - '0'; }
      v = u <= INT64_MAX ? new(_a.alloc()) Json::_H(static_cast<i64>(u)) : new(_a.alloc()) Json::_H(u); return --p;
    }
  $:// { double d; if (str2double(b, d)) { v = new(_a.alloc()) Json::_H(d); return p - 1; } return 0; }
    {
      c = errno; double _ = strtod(b, const_cast<char**>(&e));
      if (errno != ERANGE) { v = new(_a.alloc()) Json::_H(_); return --e; } errno = c;
      if (_ != HUGE_VAL && _ != -HUGE_VAL) { v = new(_a.alloc()) Json::_H(_); return --e; }
    }
    return 0;
  }
  bool Parser::parse_comments(const char* b, const char* e, void*& val) {
    union { u32 state; void* pstate; }; union { u32 size;  void* psize; }; void* key; const char* p; size_t l = 0; char m;
    xx::Array& s = _a._stack; xx::Array& u = _a._ustack; state = size = 0; skip_white_space(b, e);
  obj_txt:
    switch (*b) {
    case '/': ++b; if (*b == '/') { while (*++b) { if (*b == 0xA) { ++b; break; } } skip_white_space(b, e); goto obj_txt; }
            if (*b == 0x2A) { while (*++b) { if (*b == 0x2A) { if (*++b == 0x2F) { skip_white_; goto obj_txt; } } } } break;
    case '{': goto obj_beg; case '[': goto arr_beg; case '"': b = parse_string(++b, e, val); break;
    case 'f': b = parse_false(b, e, val); break; case 't': b = parse_true(b, e, val); break;
    case 'n': b = parse_null(b, e, val); break; case '\0': return false; default: b = parse_number(b, e, val);
    }
    if (b == 0) goto err; goto end;
  obj_beg:
    u.push_back(psize); u.push_back(pstate); s.push_back(make_object(_a)); size = s.size(); state = '{';
  obj_val_beg:
    skip_white_; m = *b; if (m == '}') goto val_end;
    if (m == '/') {
      ++b; if (*b == '/') { while (*++b) { if (*b == 0xA) { ++b; if (*b == '}') goto val_end; --b; goto obj_val_beg; } } goto err; } if (*b == 0x2A) {
        while (*++b) { if (*b == 0x2A) { if (*++b == 0x2F) goto obj_val_beg; } } goto err;
      }
    }
    if (*b != '"') goto err; ++b; p = static_cast<const char*>(memchr(b, '"', e - b)); $: l += p - b - l;
    if (b[l - 1] == '\\') { ++l; p = static_cast<const char*>(memchr(b + l, '"', e - b - l)); goto $; }
    if (p) { key = make_key(_a, b, l); } else goto err; b = p; s.push_back(key); skip_white_;
    if (*b != ':') goto err; skip_white_;
    switch (*b) {
    case '"': b = parse_string(++b, e, val); break; case '{': goto obj_beg; case '[': goto arr_beg;
    case 'f': b = parse_false(b, e, val); break; case 't': b = parse_true(b, e, val); break;
    case 'n': b = parse_null(b, e, val); break; case '\0': goto err; default: b = parse_number(b, e, val);
    }
    if (b == 0) goto err; s.push_back(val);
  obj_val_end:
    skip_white_; m = *b; if (m == ',') goto obj_val_beg; if (m == '}' || m == ']') goto val_end;
    if (m == '/') {
      ++b; if (*b == '/') { while (*++b) { if (*b == 0xA) { ++b; if (*b == '}') goto val_end; --b; goto obj_val_end; } } goto err; } if (*b == 0x2A) {
        while (*++b) { if (*b == 0x2A) { if (*++b == 0x2F) goto obj_val_end; } }
      }
    }
    goto err;
  arr_beg:
    u.push_back(psize);  // prev size
    u.push_back(pstate); // prev state
    s.push_back(make_array(_a));
    size = s.size(); // current size
    state = '[';
  arr_val_beg:
    skip_white_; m = *b; if (m == ']') goto val_end;
    if (m == '/') {
      ++b; if (*b == '/') { while (*++b) { if (*b == 0xA) { ++b; if (*b == ']') goto val_end; --b; goto arr_val_beg; } } goto err; } if (*b == 0x2A) {
        while (*++b) { if (*b == 0x2A) { if (*++b == 0x2F) goto arr_val_beg; } } goto err;
      }
    }
    switch (*b) {
    case '"': b = parse_string(++b, e, val); break; case '{': goto obj_beg; case '[': goto arr_beg;
    case 'f': b = parse_false(b, e, val); break; case 't': b = parse_true(b, e, val); break;
    case 'n': b = parse_null(b, e, val); break; case '\0': goto err; default: b = parse_number(b, e, val);
    }
    if (b == 0) goto err; s.push_back(val);
  arr_val_end:
    skip_white_; m = *b; if (m == ',') goto arr_val_beg; if (m == ']') goto val_end;
    if (m == '/') {
      ++b; if (*b == '/') { while (*++b) { if (*b == 0xA) { ++b; if (*b == ']') goto val_end; --b; goto obj_val_end; } } goto err; } if (*b == 0x2A) {
        while (*++b) { if (*b == 0x2A) { if (*++b == 0x2F) goto obj_val_end; } }
      }
    }
    goto err;
  val_end:
    if (s.size() > size) {
      void* p = xx::alloc_array(s.data() + size, s.size() - size); s.resize(size); static_cast<Json::_H*>(s.back())->p = p;
    }
    pstate = u.pop_back();
    if (state == '{') { psize = u.pop_back(); goto obj_val_end; } if (state == '[') { psize = u.pop_back(); goto arr_val_end; }
    u.pop_back(); val = s.pop_back();
  end:skip_white_; return b == e;
  err:
    while (s.size() > 0) {
      if (s.size() > size) {
        if (state == '{' && ((s.size() - size) & 1)) s.push_back(0); void* p = xx::alloc_array(s.data() + size, s.size() - size);
        s.resize(size); static_cast<Json::_H*>(s.back())->p = p;
      }
      pstate = u.pop_back(); psize = u.pop_back(); if (state == 0) { val = s.pop_back(); break; }
    }
    return false;
  }
  //Parsing with comments (usually used to read configuration files)
  bool Json::parse(const char* s, size_t n) {
    Parser parser; bool r = parser.parse_comments(s, s + n, *(void**)&_h); if (_unlikely(!r && _h)) this->reset(); return r;
  }
  _FORCE_INLINE const char* find_escapse(const char* b, const char* e, char& c) {
#if 1
    char c0, c1, c2, c3, c4, c5, c6, c7;
    for (;;) {
      if (b + 8 <= e) {
        if ((c0 = xx::e2s_table[static_cast<u8>(b[0])])) { c = c0; return b; }
        if ((c1 = xx::e2s_table[static_cast<u8>(b[1])])) { c = c1; return b + 1; }
        if ((c2 = xx::e2s_table[static_cast<u8>(b[2])])) { c = c2; return b + 2; }
        if ((c3 = xx::e2s_table[static_cast<u8>(b[3])])) { c = c3; return b + 3; }
        if ((c4 = xx::e2s_table[static_cast<u8>(b[4])])) { c = c4; return b + 4; }
        if ((c5 = xx::e2s_table[static_cast<u8>(b[5])])) { c = c5; return b + 5; }
        if ((c6 = xx::e2s_table[static_cast<u8>(b[6])])) { c = c6; return b + 6; }
        if ((c7 = xx::e2s_table[static_cast<u8>(b[7])])) { c = c7; return b + 7; }
        b += 8;
      } else {
        if (b + 4 <= e) {
          if ((c0 = xx::e2s_table[static_cast<u8>(b[0])])) { c = c0; return b; }
          if ((c1 = xx::e2s_table[static_cast<u8>(b[1])])) { c = c1; return b + 1; }
          if ((c2 = xx::e2s_table[static_cast<u8>(b[2])])) { c = c2; return b + 2; }
          if ((c3 = xx::e2s_table[static_cast<u8>(b[3])])) { c = c3; return b + 3; }
          b += 4;
        }
        for (; b < e; ++b) {
          if ((c = xx::e2s_table[static_cast<u8>(*b)])) return b;
        }
        return e;
      }
    }
#else
    for (; b < e; ++b) {
      if ((c = xx::e2s_table[static_cast<u8>(*b)])) return b;
    }
    return e;
#endif
  }
  std::string& Json::str(std::string& fs, int mdp) const {
    if (!_h) return fs.append("null", 4);
    switch (_h->type) {
    case t_string: {
      fs << '"';
      const u32 len = _h->size;
      const char* s = _h->s;
      const char* e = s + len;
      char c;
      for (const char* p; (p = find_escapse(s, e, c)) < e;) {
        fs.append(s, p - s).append({ '\\', c });
        s = p + 1;
      }
      if (s != e) fs.append(s, e - s);
      fs << '"';
      break;
    }
    case t_object: {
      fs << '{';
      if (_h->p) {
        xx::Array& a = *(xx::Array*)&_h->p;
        for (u32 i = 0; i < a.size(); i += 2) {
          fs << '"' << static_cast<const char*>(a[i]) << '"' << ':';
          ((Json*)&a[i + 1])->str(fs, mdp) << ',';
        }
      }
      fs.back() == ',' ? (void)(fs.back() = '}') : (void)(fs.append({ '}' }));
      break;
    }
    case t_array: {
      fs << '[';
      if (_h->p) {
        xx::Array& a = *(xx::Array*)&_h->p;
        for (u32 i = 0; i < a.size(); ++i) {
          ((Json*)&a[i])->str(fs, mdp) << ',';
        }
      }
      fs.back() == ',' ? (void)(fs.back() = ']') : (void)(fs.append({ ']' }));
      break;
    }
    case t_int: fs << _h->i; break;
    case t_uint: fs << static_cast<u64>(_h->i); break;
    case t_bool: fs << _h->b; break;
    case t_double: fs << _h->d; break;
    }
    return fs;
  }
  // @indent:  4 spaces by default
  // @n:       number of spaces to insert at the beginning for the current line
  std::string& Json::_json2pretty(std::string& fs, int indent, int n, int mdp) const {
    if (!_h) return fs.append("null", 4);
    switch (_h->type) {
    case t_object: {
      fs << '{';
      if (_h->p) {
        xx::Array& a = *(xx::Array*)&_h->p;
        for (u32 i = 0; i < a.size(); i += 2) {
          fs.append({ '\n' }).append(n, ' ') << '"' << static_cast<const char*>(a[i]) << '"' << ": ";
          ((Json*)&a[i + 1])->_json2pretty(fs, indent, n + indent, mdp) << ',';
        }
      }
      if (fs.back() == ',') {
        fs.back() = '\n'; if (n > indent) fs.append(n - indent, ' ');
      }
      fs << '}'; break;
    }
    case t_array: {
      fs << '[';
      if (_h->p) {
        xx::Array& a = *(xx::Array*)&_h->p;
        for (u32 i = 0; i < a.size(); ++i) {
          fs.append({ '\n' }).append(n, ' '); ((Json*)&a[i])->_json2pretty(fs, indent, n + indent, mdp) << ',';
        }
      }
      if (fs.back() == ',') {
        fs.back() = '\n'; if (n > indent) fs.append(n - indent, ' ');
      }
      fs << ']'; break;
    }
    default: str(fs, mdp);
    }
    return fs;
  }
  bool Json::has_member(const char* key) const {
    if (this->is_object()) {
      for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
        if (strcmp(key, it.key()) == 0) return true;
      }
    }
    return false;
  }
  Json& Json::operator[](const char* key) const {
    assert(!_h || _h->type & t_object);
    for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
      if (strcmp(key, it.key()) == 0) return it.value();
    }
    if (!_h) {
      ((Json*)this)->_h = make_object(xx::jalloc()); new (&_h->p) xx::Array(8);
    }
    if (!_h->p) new (&_h->p) xx::Array(8);
    xx::Array& a = _array(); a.push_back(make_key(xx::jalloc(), key)); a.push_back(0);
    return *(Json*)&a.back();
  }
  Json& Json::operator[](const std::string_view& key) const {
    for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
      if (strcmp(key.data(), it.key()) == 0) return it.value();
    }
    if (!_h) {
      ((Json*)this)->_h = make_object(xx::jalloc()); new (&_h->p) xx::Array(8);
    }
    if (!_h->p) new (&_h->p) xx::Array(8); xx::Array& a = _array();
    a.push_back(make_key(xx::jalloc(), key)); a.push_back(0);
    return *(Json*)&a.back();
  }
  Json& Json::get(u32 i) const {
    if (this->is_array() && _array().size() > i) {
      return *(Json*)&_array()[i];
    }
    return xx::_null;
  }
  Json& Json::get(const char* key) const {
    if (this->is_object()) {
      for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
        if (strcmp(key, it.key()) == 0) return it.value();
      }
    }
    return xx::_null;
  }
  void Json::remove(const char* key) {
    if (this->is_object()) {
      const u32 n = _h->p ? _array().size() : 0;
      if (n > 0) {
        xx::Array& a = _array();
        for (u32 i = 0; i < n; i += 2) {
          const char* s = static_cast<const char*>(a[i]);
          if (strcmp(key, s) == 0) {
            xx::jalloc().free(static_cast<void*>(const_cast<char*>(s)), static_cast<u32>(strlen(s)) + 1);
            ((Json&)a[i + 1]).reset();
            a.remove_pair(i);
            return;
          }
        }
      }
    }
  }
  void Json::erase(const char* key) {
    if (this->is_object()) {
      const u32 n = _h->p ? _array().size() : 0;
      if (n > 0) {
        xx::Array& a = _array();
        for (u32 i = 0; i < n; i += 2) {
          const char* s = static_cast<const char*>(a[i]);
          if (strcmp(key, s) == 0) {
            xx::jalloc().free(static_cast<void*>(const_cast<char*>(s)), static_cast<u32>(strlen(s)) + 1);
            ((Json&)a[i + 1]).reset();
            a.erase_pair(i);
            return;
          }
        }
      }
    }
  }
  Json& Json::_set(u32 i) {
  beg:
    if (this->_h == 0) {
      for (u32 k = 0; k < i; ++k) {
        this->push_back(Json());
      }
      this->push_back(Json());
      return *(Json*)&_array()[i];
    }
    if (_unlikely(!this->is_array())) {
      this->reset();
      goto beg;
    }
    xx::Array& a = _array();
    if (i < a.size()) {
      return *(Json*)&a[i];
    } else {
      for (u32 k = a.size(); k < i; ++k) {
        this->push_back(Json());
      }
      this->push_back(Json());
      return *(Json*)&_array()[i]; // don't use `a` here
    }
  }
  Json& Json::_set(const char* key) {
  beg:
    if (this->_h == 0) {
      this->add_member(key, Json());
      return *(Json*)&_array()[1];
    }
    if (_unlikely(!this->is_object())) {
      this->reset();
      goto beg;
    }
    for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
      if (strcmp(key, it.key()) == 0) return it.value();
    }
    this->add_member(key, Json());
    return *(Json*)&_array().back();
  }
  void Json::reset() {
    if (_h) {
      xx::Alloc& a = xx::jalloc();
      switch (_h->type) {
      case t_object:
        for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
          a.free(static_cast<void*>(const_cast<char*>(it.key())), static_cast<u32>(strlen(it.key())) + 1);
          it.value().reset();
        }
        if (_h->p) _array().~Array();
        break;
      case t_array:
        for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
          (*it).reset();
        }
        if (_h->p) _array().~Array();
        break;
      case t_string:
        if (_h->s) a.free(_h->s, _h->size + 1);
        break;
      }
      a.free(_h);
      _h = 0;
    }
  }
  void* Json::_dup() const {
    Json::_H* h = 0;
    if (_h) {
      switch (_h->type) {
      case t_object:
        h = make_object(xx::jalloc());
        if (_h->p) {
          xx::Array* a = new(&h->p) xx::Array(_array().size());
          for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
            a->push_back(make_key(xx::jalloc(), it.key()));
            a->push_back(it.value()._dup());
          }
        }
        break;
      case t_array:
        h = make_array(xx::jalloc());
        if (_h->p) {
          xx::Array* a = new(&h->p) xx::Array(_array().size());
          for (Json::iterator it = this->begin(); it != iterator::end(); ++it) {
            a->push_back((*it)._dup());
          }
        }
        break;
      case t_string:
        h = make_string(xx::jalloc(), _h->s, _h->size);
        break;
      default:
        h = static_cast<Json::_H*>(xx::jalloc().alloc());
        h->type = _h->type;
        h->i = _h->i;
      }
    }
    return h;
  }
  Json::Json(std::initializer_list<Json> v) {
    const bool is_obj = std::all_of(v.begin(), v.end(), [](const Json& x) {
      return x.is_array() && x.array_size() == 2 && x[0].is_string();
      });
    if (is_obj) {
      _h = make_object(xx::jalloc());
      const u32 n = static_cast<u32>(v.size() << 1);
      if (n > 0) {
        xx::Array* a = new(&_h->p) xx::Array(n);
        for (const Json& x : v) {
          a->push_back(x[0]._h->s); x[0]._h->s = 0; a->push_back(x[1]._h); x[1]._h = 0;
        }
      }
    } else {
      _h = make_array(xx::jalloc());
      const u32 n = static_cast<u32>(v.size());
      if (n > 0) {
        xx::Array* a = new(&_h->p) xx::Array(n);
        for (const Json& x : v) {
          a->push_back(x._h); ((Json*)&x)->_h = 0;
        }
      }
    }
  }
  Json array(std::initializer_list<Json> v) {
    Json r(json::array());
    const u32 n = static_cast<u32>(v.size());
    if (n > 0) {
      xx::Array* a = new(&(*reinterpret_cast<Json::_H**>(&r))->p) xx::Array(n);
      for (const Json& x : v) {
        a->push_back(*reinterpret_cast<Json::_H**>(&const_cast<Json&>(x)));
        *reinterpret_cast<Json::_H**>(&const_cast<Json&>(x)) = 0;
      }
    }
    return r;
  }
  Json object(std::initializer_list<Json> v) {
    Json r(json::object());
    const u32 n = static_cast<u32>(v.size() << 1);
    if (n > 0) {
      xx::Array* a = new(&(*reinterpret_cast<Json::_H**>(&r))->p) xx::Array(n);
      for (const Json& x : v) {
        assert(x.is_array() && x.size() == 2 && x[0].is_string());
        a->push_back((*(Json::Json::_H**)&x[0])->s);
        (*reinterpret_cast<Json::_H**>(&x[0]))->s = 0;
        a->push_back(*reinterpret_cast<Json::_H**>(&x[1]));
        *reinterpret_cast<Json::_H**>(&x[1]) = 0;
      }
    }
    return r;
  }
} // json