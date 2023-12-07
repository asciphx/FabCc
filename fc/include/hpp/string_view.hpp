#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP
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
#include <tp/c++.h>
#include <cstring>
#include <string>
#if (defined(_HAS_CXX17) && _HAS_CXX17 == 1) || (defined(__cplusplus) && __cplusplus >= 201703L)
#include <string_view>
#else//__cplusplus <= 201402L
#include <stdexcept>
#include <ostream>
#include <algorithm>
#include <memory>
#include <iterator>
#include <exception>
#include <istream>
namespace std {
  class string_view {
    const char* data_; size_t length_;
  public:
    using size_type = size_t;
    static constexpr size_t npos = size_t(-1);
    constexpr string_view() noexcept: data_(nullptr), length_(0) {}
    template<size_t T>
    constexpr string_view(const char(&data)[T]): data_{ data }, length_{ T - 1 } {}
    explicit constexpr string_view(const char* data, const size_t length) : data_(data), length_(length) {}
    string_view(const char* data): data_(data), length_(char_traits<char>::length(data)) {}
    constexpr string_view(const string_view&) noexcept = default;
    string_view(const basic_string<char, char_traits<char>, allocator<char>>& s) noexcept
      : data_(s.data()), length_(s.length()) {}
    _FORCE_INLINE string_view& operator=(const string_view& view) noexcept {
      data_ = view.data(); length_ = view.size(); return *this;
    }
    explicit operator basic_string<char, char_traits<char>, allocator<char>>() const {
      return basic_string<char, char_traits<char>, allocator<char>>(data_, length_);
    }
    _FORCE_INLINE const char* begin() const noexcept { return data_; }
    _FORCE_INLINE const char* end() const noexcept { return data_ + length_; }
    _FORCE_INLINE size_t size() const noexcept { return length_; }
    _FORCE_INLINE bool empty() const noexcept { return length_ == 0; }
    _FORCE_INLINE const char& operator[](size_t o) const { return data_[o]; }
    _FORCE_INLINE const char& at(size_t o) const {
      if (o >= length_) { throw out_of_range("o exceeds length"); } return data_[o];
    }
    _FORCE_INLINE const char& front() const noexcept { return data_[0]; }
    _FORCE_INLINE const char& back()  const noexcept { return data_[length_ - 1]; }
    _FORCE_INLINE const char* data() const noexcept { return data_; }
    _FORCE_INLINE string_view substr(size_t a) const noexcept {
      if (a >= length_) { return string_view(); } return string_view(data_ + a, length_ - a);
    }
    _FORCE_INLINE string_view substr(size_t a, size_t b) const noexcept {
      if (a >= length_) { return string_view(); } return string_view(data_ + a, a + b > length_ ? length_ - a : b);
    }
    _FORCE_INLINE size_t find(const char c) const noexcept {
      size_t l = 0; while (l < length_) { if (data_[l] == c) { return l; } ++l; } return this->npos;
    }
    _FORCE_INLINE size_t find(const std::string& c) const noexcept {
      size_t l = 0, a = 0; while (l < length_) {
        if (data_[l] != c[a])a = 0; ++l; ++a; if (a == c.length()) { return l - c.length(); }
      } return this->npos;
    }
    _FORCE_INLINE size_t find(const std::string_view& c) const noexcept {
      size_t l = 0, a = 0; while (l < length_) {
        if (data_[l] != c[a])a = 0; ++l; ++a; if (a == c.size()) { return l - c.size(); }
      } return this->npos;
    }
    _FORCE_INLINE size_t find(const char* c) const noexcept {
      size_t l = 0, L = strlen(c), a = 0; while (l < length_) {
        if (data_[l] != c[a])a = 0; ++l; ++a; if (a == L) { return l - L; }
      } return this->npos;
    }
    _FORCE_INLINE size_t find(const char* c, size_t L) const noexcept {
      size_t l = 0, a = 0; while (l < length_) {
        if (data_[l] != c[a])a = 0; ++l; ++a; if (a == L) { return l - L; }
      } return this->npos;
    }
    _FORCE_INLINE size_t rfind(const char c) const noexcept {
      size_t s = length_; while (0 < s) { if (data_[s] == c) { return s; } --s; } return this->npos;
    }
    size_t rfind(const char* s) const noexcept {
      size_t m = strlen(s); if (m == 1) return this->rfind(*s);
      size_t n = this->size(); if (n < m) return this->npos; size_t c[256] = { 0 };
      for (size_t i = m; i > 0; --i) c[s[i - 1]] = i;
      for (size_t j = n - m;;) {
        if (memcmp(s, data_ + j, m) == 0) return j;
        if (j == 0) return this->npos; size_t x = c[data_[j - 1]];
        if (x == 0) x = m + 1; if (j < x) return this->npos; j -= x;
      }
    }
    _FORCE_INLINE size_t find_first_of(const basic_string<char, char_traits<char>, allocator<char>>& s, size_t l = 0) const noexcept {
      size_t a = 0; while (l < length_) { while (s[a]) { if (data_[l] == s[a]) return l; ++a; } ++l; a = 0; } return this->npos;
    }
    _FORCE_INLINE size_t find_first_of(const char* s, size_t l = 0) const noexcept {
      size_t a = 0; while (l < length_) { while (s[a]) { if (data_[l] == s[a]) return l; ++a; } ++l; a = 0; } return this->npos;
    }
    _FORCE_INLINE size_t find_first_of(char c, size_t l = 0) const noexcept {
      while (l < length_) { if (data_[l] == c) return l; ++l; } return this->npos;
    }
    _FORCE_INLINE size_t find_first_not_of(const basic_string<char, char_traits<char>, allocator<char>>& s) const {
      if (this->empty()) return this->npos; size_t r = strspn(this->data(), s.data()); return data_[r] ? r : this->npos;
    }
    _FORCE_INLINE size_t find_first_not_of(const basic_string<char, char_traits<char>, allocator<char>>& s, size_t o) const {
      if (this->size() <= o) return this->npos; size_t r = strspn(this->data() + o, s.data()) + o; return data_[r] ? r : this->npos;
    }
    _FORCE_INLINE size_t find_first_not_of(const char* s) const {
      if (this->empty()) return this->npos; size_t r = strspn(this->data(), s); return data_[r] ? r : this->npos;
    }
    _FORCE_INLINE size_t find_first_not_of(const char* s, size_t o) const {
      if (this->size() <= o) return this->npos; size_t r = strspn(this->data() + o, s) + o; return data_[r] ? r : this->npos;
    }
    _FORCE_INLINE size_t find_first_not_of(char c, size_t o = 0) const {
      if (this->size() <= o) return this->npos; char s[2] = { c, '\0' }; size_t r = strspn(this->data() + o, s) + o; return data_[r] ? r : this->npos;
    }
    [[nodiscard]] int compare(const string_view& s) const noexcept {
      const int rc = char_traits<char>::compare(data_, s.data_, (min)(length_, s.length_));
      return rc != 0 ? rc : (length_ == s.length_ ? 0 : length_ < s.length_ ? -1 : 1);
    }
    [[nodiscard]] int compare(const char* data) const noexcept {
      const size_t l = char_traits<char>::length(data);
      const int rc = char_traits<char>::compare(data_, data, (min)(length_, l));
      return rc != 0 ? rc : (length_ == l ? 0 : length_ < l ? -1 : 1);
    }
    [[nodiscard]] int compare(const basic_string<char, char_traits<char>, allocator<char>>& s) const noexcept {
      const int rc = char_traits<char>::compare(data_, s.data(), (min)(length_, s.length()));
      return rc != 0 ? rc : (length_ == s.length() ? 0 : length_ < s.length() ? -1 : 1);
    }
    friend basic_ostream<char>& operator<<(basic_ostream<char>& os, const string_view& sv) {
      os.write(sv.data_, sv.length_); return os;
    }
    friend string& operator+=(string& s, const string_view& _v) { s.append(_v.data_, _v.length_); return s; };
  };
  // ==
  _FORCE_INLINE bool operator==(const string_view& l, const string_view& r) noexcept {
    return l.compare(r) == 0;
  }
  _FORCE_INLINE bool operator==(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
    return l.compare(r) == 0;
  }
  _FORCE_INLINE bool operator==(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
    return r.compare(l) == 0;
  }
  _FORCE_INLINE bool operator==(const string_view& l, const char* r) noexcept {
    return l.compare(r) == 0;
  }
  _FORCE_INLINE bool operator==(const char* l, const string_view& r) noexcept {
    return r.compare(l) == 0;
  }
  // !=
  _FORCE_INLINE bool operator!=(const string_view& l, const string_view& r) noexcept {
    return l.compare(r) != 0;
  }
  _FORCE_INLINE bool operator!=(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
    return l.compare(r) != 0;
  }
  _FORCE_INLINE bool operator!=(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
    return r.compare(l) != 0;
  }
  _FORCE_INLINE bool operator!=(const string_view& l, const char* r) noexcept {
    return l.compare(r) != 0;
  }
  _FORCE_INLINE bool operator!=(const char* l, const string_view& r) noexcept {
    return r.compare(l) != 0;
  }
  // <=
  _FORCE_INLINE bool operator<=(const string_view& l, const string_view& r) noexcept {
    return l.compare(r) <= 0;
  }
  _FORCE_INLINE bool operator<=(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
    return l.compare(r) <= 0;
  }
  _FORCE_INLINE bool operator<=(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
    return r.compare(l) >= 0;
  }
  // <
  _FORCE_INLINE bool operator<(const string_view& l, const string_view& r) noexcept {
    return l.compare(r) < 0;
  }
  _FORCE_INLINE bool operator<(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
    return l.compare(r) < 0;
  }
  _FORCE_INLINE bool operator<(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
    return r.compare(l) > 0;
  }
  // >=
  _FORCE_INLINE bool operator>=(const string_view& l, const string_view& r) noexcept {
    return l.compare(r) >= 0;
  }
  _FORCE_INLINE bool operator>=(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
    return l.compare(r) >= 0;
  }
  _FORCE_INLINE bool operator>=(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
    return r.compare(l) <= 0;
  }
  // >
  _FORCE_INLINE bool operator>(const string_view& l, const string_view& r) noexcept {
    return l.compare(r) > 0;
  }
  _FORCE_INLINE bool operator>(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
    return l.compare(r) > 0;
  }
  _FORCE_INLINE bool operator>(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
    return r.compare(l) < 0;
  }
  template<>
  struct hash<string_view> {
    [[nodiscard]] size_t operator()(const string_view _) const noexcept {
#ifdef _MSVC_LANG
      return std::_Hash_array_representation(_.data(), _.size());
#else
      return std::_Hash_impl::hash(_.data(), _.size());
#endif // _WIN32
    }
  };
} // namespace std
#endif
#include <hpp/i2a.hpp>
#include <h/dtoa_milo.h>
namespace std {
  //template <class T> struct is_char: std::false_type {}; template <> struct is_char<std::string>: std::true_type {};
  //template <> struct is_char<const char*>: std::true_type {}; template <> struct is_char<text<>>: std::true_type {};
  static const string STD_TURE("true", 4), STD_FALSE("false", 5);
  template <typename T> inline void del(T* p) { if (p) { p->~T(); ::free((void*)p); } }
  template <typename T, typename... Args>
  inline T* make(Args&&... args) { return new (malloc(sizeof(T))) T(std::forward<Args>(args)...); }
  _FORCE_INLINE string& operator<<(string& s, const char* _v) { s.append(_v); return s; };
  _FORCE_INLINE string& operator<<(string& s, const string& _v) { s.append(_v.data(), _v.size()); return s; };
  _FORCE_INLINE string& operator<<(string& s, const string_view& _v) { s.append(_v.data(), _v.size()); return s; };
  _FORCE_INLINE string& operator<<(string& s, int _) { char c[11]; return s.append(c, i2a(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, unsigned int _) { char c[10]; return s.append(c, u2a(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, short _) { char c[6]; return s.append(c, s2a(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, unsigned short _) { char c[5]; return s.append(c, t2a(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, signed char _) { char c[3]; return s.append(c, a2a(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, unsigned char _) { char c[3]; return s.append(c, h2a(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, char _) { s.append({ _ }); return s; };
  _FORCE_INLINE string& operator<<(string& s, bool b) { if (b) return s += STD_TURE; return s += STD_FALSE; }
  _FORCE_INLINE string& operator<<(string& s, long& i) { char c[20]; return s.append(c, i64toa(c, i) - c); }
  _FORCE_INLINE string& operator<<(string& s, long unsigned& i) { char c[20]; return s.append(c, u64toa(c, i) - c); }
  _FORCE_INLINE string& operator<<(string& s, double& f) { char c[20]; return s.append(c, milo::dtoa(f, c, 0x10)); }
  _FORCE_INLINE string& operator<<(string& s, float& f) { char c[10]; return s.append(c, milo::dtoa(f, c, 0x7)); }
  _FORCE_INLINE string& operator<<(string& s, long double& f) { string k(to_string(f)); return s.append(k.data(), k.size()); }
  _FORCE_INLINE string& operator<<(string& s, long long& _) { char c[20]; return s.append(c, i64toa(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, unsigned long long& _) { char c[20]; return s.append(c, u64toa(c, _)); };
  _FORCE_INLINE string& operator<<(string& s, long&& i) { char c[20]; return s.append(c, i64toa(c, std::move(i)) - c); }
  _FORCE_INLINE string& operator<<(string& s, long unsigned&& i) { char c[20]; return s.append(c, u64toa(c, std::move(i)) - c); }
  _FORCE_INLINE string& operator<<(string& s, double&& f) { char c[20]; return s.append(c, milo::dtoa(std::move(f), c, 0x10)); }
  _FORCE_INLINE string& operator<<(string& s, float&& f) { char c[10]; return s.append(c, milo::dtoa(std::move(f), c, 0x7)); }
  _FORCE_INLINE string& operator<<(string& s, long double&& f) { string k(to_string(f)); return s.append(k.data(), k.size()); }
  _FORCE_INLINE string& operator<<(string& s, long long&& _) { char c[20]; return s.append(c, i64toa(c, std::move(_))); };
  _FORCE_INLINE string& operator<<(string& s, unsigned long long&& _) { char c[20]; return s.append(c, u64toa(c, std::move(_))); };
  static std::string strip(const char* s, const char* c, char d = 'b') {
    if (_unlikely(!*s)) return std::string();
    char bs[256] = { 0 }; while (*c) bs[(const u8)(*c++)] = 1;
    if (d == 'l' || d == 'L') {
      while (bs[(u8)(*s)]) ++s; return std::string(s);
    } else if (d == 'r' || d == 'R') {
      const char* e = s + strlen(s) - 1;
      while (e >= s && bs[(u8)(*e)]) --e; return std::string(s, e + 1 - s);
    } else {
      while (bs[(u8)(*s)]) ++s; const char* e = s + strlen(s) - 1;
      while (e >= s && bs[(u8)(*e)]) --e; return std::string(s, e + 1 - s);
    }
  }
  static std::string strip(const std::string& s, const char* c = " \t\r\n", char d = 'b') {
    if (_unlikely(s.empty())) return std::string(); char bs[256] = { 0 };
    while (*c) bs[(const u8)(*c++)] = 1;
    if (d == 'l' || d == 'L') {
      size_t b = 0; while (b < s.size() && bs[(u8)(s[b])]) ++b;
      return b == 0 ? s : s.substr(b);
    } else if (d == 'r' || d == 'R') {
      size_t e = s.size(); while (e > 0 && bs[(u8)(s[e - 1])]) --e;
      return e == s.size() ? s : s.substr(0, e);
    } else {
      size_t b = 0, e = s.size(); while (b < s.size() && bs[(u8)(s[b])]) ++b;
      if (b == s.size()) return std::string();
      while (e > 0 && bs[(u8)(s[e - 1])]) --e;
      return (e - b == s.size()) ? s : s.substr(b, e - b);
    }
  }
  // split a string, starting from cur && ending with split_char. Advance cur to the end of the split.
  static std::string_view split(const char*& cur, const char* line_end, char split_char) {
    const char* start = cur; while (start < (line_end - 1) && *start == split_char) ++start;
    const char* end = start + 1; while (end < (line_end - 1) && *end != split_char) ++end;
    cur = end + 1; if (*end == split_char) return std::string_view(start, cur - start - 1);
    else return std::string_view(start, cur - start);
  }
  static std::vector<std::string> split(const std::string& s, char c, u32 max_num = 0) {
    std::vector<std::string> v; v.reserve(8); const char* p, * from = s.data();
    while ((p = (const char*)memchr(from, c, s.data() + s.size() - from))) {
      v.push_back(std::string(from, p - from)); from = p + 1; if (v.size() == max_num) break;
    }
    if (from < s.data() + s.size()) v.push_back(std::string(from, s.data() + s.size() - from)); return v;
  }
  static std::string replace(const char* s, const char* sub, const char* to, u32 max_num = 0) {
    const char* p, * from = s; size_t n = strlen(sub), m = strlen(to); std::string x;
    while ((p = strstr(from, sub))) {
      x.append(from, p - from); x.append(to, (unsigned int)m); from = p + n;
      if (--max_num == 0) break;
    }
    if (from < s + strlen(s)) x.append(from); return x;
  }
  static std::string replace(const std::string& s, const char* sub, const char* to, u32 max_num = 0) {
    const char* from = s.c_str(), * p = strstr(from, sub); if (!p) return s;
    size_t n = strlen(sub), m = strlen(to); std::string x(s.size(), '\0'); x.clear();
    do {
      x.append(from, p - from).append(to, (unsigned int)m); from = p + n;
      if (--max_num == 0) break;
    } while ((p = strstr(from, sub)));
    if (from < s.data() + s.size()) x.append(from); return x;
  }
  static _FORCE_INLINE bool starts_with(const std::string& $, const char* s) {
    size_t n = strlen(s); if (n == 0) return true; return n <= $.size() && memcmp($.data(), s, n) == 0;
  }
  static _FORCE_INLINE bool ends_with(const std::string& $, const char* s) {
    size_t n = strlen(s); if (n == 0) return true; return n <= $.size() && memcmp($.data() + ($.size() - n), s, n) == 0;
  }
  static std::string& replace(std::string& $, const char* sub, const char* to, size_t max_num = 0) {
    if ($.empty()) return $;
    const char* from = $.c_str(), * p = strstr(from, sub), * e = $.c_str() + $.size();
    if (!p) return $;
    size_t n = strlen(sub), m = strlen(to);
    std::string s($.size(), '\0'); s.clear();
    do {
      s.append(from, p - from).append(to, m); from = p + n;
      if (max_num && --max_num == 0) break;
    } while ((p = strstr(from, sub)));
    if (from < e) s.append(from); $.swap(s); return $;
  }
  static std::string& strip(std::string& $, const char* s = " \t\r\n", char d = 'b') {
    if ($.empty()) return $;
    char bs[0x100] = { 0 };
    while (*s) bs[(const unsigned char)(*s++)] = 1;
    size_t e = 0; char* c = const_cast<char*>($.data()) + $.size();
    if (d == 'l' || d == 'L') {
      while (e < $.size() && bs[(unsigned char)($.data()[e])]) ++e;
      if (e != 0 && (c -= e) != 0) memmove(const_cast<char*>($.data()), $.data() + e, $.size());
    } else if (d == 'r' || d == 'R') {
      size_t _s = e = $.size();
      while (e > 0 && bs[(unsigned char)($.data()[e - 1])]) --e;
      if (e != _s) c = const_cast<char*>($.data()) + e;
    } else {
      size_t _s = e = $.size();
      while (e > 0 && bs[(unsigned char)($.data()[e - 1])]) --e;
      if (e != _s) c = const_cast<char*>($.data()) + e;
      if (e == 0) return $; e = 0;
      while (e < _s && bs[(unsigned char)($.data()[e])]) ++e;
      if (e != 0 && (_s -= e) != 0) memmove(const_cast<char*>($.data()), $.data() + e, $.size());
    }
    return $;
  }
}
#endif