#ifndef BUF_HH
#define BUF_HH
#include <cstring>
#include <string>
#include <hpp/string_view.hpp>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <h/dtoa_milo.h>
#include <tp/jeaiii.hpp>
#pragma warning(disable:4244)
#if defined(_MSC_VER) && !defined(_INLINE)
#define _INLINE __forceinline
#elif !defined(_INLINE)
#define _INLINE inline __attribute__((always_inline))
#endif
namespace fc {
  static const std::string_view RES_TURE("true", 4), RES_FALSE("false", 5);
  struct Buf {
    char* data_;
    char* end_;
    Buf();
    Buf(Buf&& o);
    Buf(const Buf& o);
    Buf(unsigned int capacity);
    Buf(const char* c, unsigned int capacity);
    Buf(const char* c);
    Buf(const std::string& s);
    Buf(const std::string_view& s);
    Buf(unsigned int n, char c);
    ~Buf();
    Buf& operator=(Buf&& o) noexcept;
    Buf& operator=(const Buf& o);
    friend std::string& operator+=(std::string& s, const Buf& _v) { s.append(_v.data_, _v.end_); return s; };
    void clear();
    _INLINE void push_back(const char c) {
      if (end_ == back_) this->enlarge((cap_ + 1) >> 1); end_[0] = c; ++end_;
    }//safe
    _INLINE Buf& pop_back() { --end_; return *this; }// safe
    _INLINE void reset() { end_ = data_; }
    _INLINE bool empty() const { return end_ == data_; };
    _INLINE unsigned int size() const { return end_ - data_; }
    _INLINE unsigned int length() const { return end_ - data_; }
    _INLINE Buf& append(unsigned int n, char c) {
      if (back_ - end_ < n) enlarge(((cap_ + 1) >> 1) + n); memset(end_, c, n); end_ += n; return *this;
    }
    _INLINE Buf& append(char c) {
      if (back_ - end_ < 1u) enlarge(((cap_ + 1) >> 1) + 1); end_[0] = c; ++end_; return *this;
    }
    _INLINE Buf& append(const char* p, unsigned int n) {
      if (back_ - end_ < n) enlarge(((cap_ + 1) >> 1) + n); memcpy(end_, p, n); end_ += n; return *this;
    }
    _INLINE Buf& append(const char* p) {
      unsigned int n = (unsigned int)strlen(p);
      if (back_ - end_ < n) enlarge(((cap_ + 1) >> 1) + n); memcpy(end_, p, n); end_ += n; return *this;
    }
    _INLINE std::string b2s() const { return std::string(data_, end_ - data_); };
    _INLINE std::string_view b2v() const { return std::string_view(data_, end_ - data_); };
    _INLINE const char* data() const { return data_; };
    _INLINE const char* c_str() const { end_[0] = 0; return data_; };
    _INLINE char& operator[](unsigned int i) const { return data_[i]; }
    _INLINE const char operator()(unsigned int i) const { return i < end_ - data_ ? data_[i] : '\0'; };
    _INLINE char& back() { return data_[end_ - data_ - 1]; }
    _INLINE char& front() { return data_[0]; }
    _INLINE bool operator==(const char* c) const {
      size_t l = size(); if (l != strlen(c)) return false; return l == 0 || memcmp(data_, c, l) == 0;
    }
    _INLINE bool operator!=(const char* c) const {
      size_t l = size(); if (l == strlen(c)) return false; return memcmp(data_, c, l) != 0;
    }
    _INLINE bool operator==(const Buf& b) const { return memcmp(data_, b.data_, this->size()) == 0; }
    _INLINE bool operator!=(const Buf& b) const { return memcmp(data_, b.data_, this->size()) != 0; }
    _INLINE bool operator<(const Buf& b) const {
      size_t i = end_ - data_, l = b.end_ - b.data_;
      if (i < l) return memcmp(data_, b.data_, i) <= 0;
      return memcmp(data_, b.data_, l) < 0;
    }
    _INLINE bool operator>(const Buf& b) const {
      size_t i = end_ - data_, l = b.end_ - b.data_;
      if (i > l) return memcmp(data_, b.data_, l) >= 0;
      return memcmp(data_, b.data_, i) > 0;
    }
    _INLINE friend std::ostream& operator<<(std::ostream& os, const Buf& s) { return os.write(s.data_, s.size()); }
    Buf substr(unsigned int a) const;
    Buf substr(unsigned int a, unsigned int b) const;
    unsigned int find(const char* c) const;
    unsigned int find(const std::string& c) const;
    unsigned int find(const char c) const;
    unsigned int find(const char* s, size_t pos) const;
    unsigned int find(char c, size_t pos) const;
    unsigned int rfind(const char c) const;
    unsigned int rfind(const char* sub) const;
    Buf& append(const Buf& s);
    void resize(unsigned int n);
    void erase(unsigned int a, unsigned int b = -1);
    void ensure(unsigned int l);
    bool reserve(unsigned int l);
    Buf& insert(char*& s, const char* e, const char* f);
    Buf& assign(const char* s, const char* e);
    void swap(Buf& fs) noexcept;
    void swap(Buf&& fs) noexcept;
    Buf& replace(const char* sub, const char* to, size_t maxreplace = 0);
    Buf& strip(const char* s = " \t\r\n", char d = 'b');
    _INLINE bool ends_with(const char* s) const {
      unsigned int n = (unsigned int)strlen(s); if (n == 0) return true; return n <= this->size() && memcmp(end_ - n, s, n) == 0;
    }
    _INLINE bool starts_with(const char* s) const {
      unsigned int n = (unsigned int)strlen(s); if (n == 0) return true; return n <= this->size() && memcmp(data_, s, n) == 0;
    }
    _INLINE unsigned int find_first_not_of(const char* s) const {
      if (this->empty()) return -1; unsigned int r = (unsigned int)strspn(this->c_str(), s); return data_[r] ? r : -1;
    }
    _INLINE unsigned int find_first_not_of(const char* s, size_t pos) const {
      if (this->size() <= pos) return -1; unsigned int r = (unsigned int)strspn(this->c_str() + pos, s) + (unsigned int)pos; return data_[r] ? r : -1;
    }
    _INLINE unsigned int find_first_not_of(char c, unsigned int pos = 0) const {
      if (this->size() <= pos) return -1; char s[2] = { c, '\0' }; unsigned int r = (unsigned int)strspn(this->c_str() + pos, s) + pos; return data_[r] ? r : -1;
    }
    Buf& operator<<(const Buf& s);
    Buf& operator<<(Buf&& s);
    _INLINE Buf& operator<<(const std::string& s) {
      if (end_ + s.size() >= back_) enlarge(((cap_ + 1) >> 1) + (unsigned int)s.size());
      memcpy(end_, s.data(), s.size()); end_ += s.size(); return *this;
    }
    _INLINE Buf& operator<<(std::string&& s) {
      if (end_ + s.size() >= back_) enlarge(((cap_ + 1) >> 1) + (unsigned int)s.size());
      memcpy(end_, s.data(), s.size()); end_ += s.size(); return *this;
    }
    _INLINE Buf& operator<<(unsigned long long& i) { char s[20]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(long long& i) { char s[20]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(int& i) { char s[11]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(long& i) { char s[11]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(unsigned long& i) { char s[10]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(unsigned int& i) { char s[10]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(short& i) { char s[5]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(unsigned short& i) { char s[4]; return this->append(s, jeaiii::to_text_from_integer(s, i) - s); }
    _INLINE Buf& operator<<(char v) { if (end_ == back_) this->enlarge((cap_ + 1) >> 1); end_[0] = v; ++end_; return *this; }
    _INLINE Buf& operator<<(bool b) { return operator<<(b ? RES_TURE : RES_FALSE); }
    _INLINE Buf& operator<<(unsigned long long&& i) { char s[20]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(long long&& i) { char s[20]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(int&& i) { char s[11]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(long&& i) { char s[11]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(unsigned long&& i) { char s[10]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(unsigned int&& i) { char s[10]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(short&& i) { char s[5]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(unsigned short&& i) { char s[4]; return this->append(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
    _INLINE Buf& operator<<(std::string_view s) {
      if (end_ + s.size() >= back_) enlarge(((cap_ + 1) >> 1) + (unsigned int)s.size());
      memcpy(end_, s.data(), s.size()); end_ += s.size(); return *this;
    }
    _INLINE Buf& operator<<(const char* s) { return this->append(s); }
    Buf& operator<<(double&& d);
    Buf& operator<<(float&& f);
    Buf& operator<<(double& d);
    Buf& operator<<(float& f);
    Buf& operator=(const char* s);
    Buf& operator=(std::string&& s);
    Buf& operator=(std::string& s);
    Buf& operator=(const std::string_view s);
    Buf& operator<<(const tm& _v);
    _INLINE bool operator==(const std::string& b) const {
      size_t t_l = this->end_ - this->data_; return t_l == b.size() && memcmp(data_, b.data(), t_l) == 0;
    }
    _INLINE bool operator==(std::string&& b) const {
      size_t t_l = this->end_ - this->data_; return t_l == b.size() && memcmp(data_, b.data(), t_l) == 0;
    }
  private:
    _INLINE void enlarge(unsigned int l) {
      char* c = static_cast<char*>(malloc(cap_)); unsigned int size = end_ - data_; memcpy(c, data_, size); delete[] data_;
      cap_ += l; data_ = new char[cap_]; end_ = data_; back_ = data_ + cap_; memcpy(data_, c, size); end_ += size; free(c);
    }
    char* back_;
    unsigned int cap_;
  };
}
namespace std {
  _INLINE bool operator==(std::string s, const fc::Buf& b) {
    size_t t_l = b.end_ - b.data_; return t_l == s.size() && memcmp(s.data(), b.data_, t_l) == 0;
  }
}
#undef _INLINE
#endif
