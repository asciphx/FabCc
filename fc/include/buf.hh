#ifndef BUF_HH
#define BUF_HH
#include <cstring>
#include <string>
#include <string_view>
#include <ostream>
#include <h/dtoa_milo.h>
#pragma warning(disable:4244)
#if defined(_MSC_VER) && !defined(_INLINE)
#define _INLINE __forceinline
#elif !defined(_INLINE)
#define _INLINE __attribute__((always_inline))
#endif
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  static const std::string_view RES_TURE("true", 4), RES_FALSE("false", 5);
  struct Buf {
	Buf();
	Buf(Buf&& o);
	Buf(const Buf& o);
	Buf(unsigned int capacity);
	Buf(const char* c, unsigned int capacity);
	Buf(const char* c);
	Buf(const std::string& s);
	Buf(unsigned int n, char c);
	~Buf();
	Buf& operator=(Buf&& o) noexcept;
	Buf& operator=(const Buf& o);
	void clear();

	_INLINE void push_back(const char c) { end_[0] = c; ++end_; }// not safe, but fast
	_INLINE Buf& pop_back() { --end_; return *this; }// safe
	_INLINE void reset() { end_ = data_; }
	_INLINE bool empty() const { return end_ == data_; };
	_INLINE unsigned int size() const { return end_ - data_; }
	_INLINE unsigned int length() const { return end_ - data_; }
	_INLINE Buf& append(unsigned int n, char c) {
	  if (back_ - end_ < n) reserve(cap_ + n); memset(end_, c, n); end_ += n; return *this;
	}
	_INLINE Buf& append(char c) {
	  if (back_ - end_ < 1u) reserve(cap_ + 1u); end_[0] = c; ++end_; return *this;
	}
	_INLINE Buf& append(const char* p, unsigned int n) {
	  if (back_ - end_ < n) reserve(cap_ + n); memcpy(end_, p, n); end_ += n; return *this;
	}
	_INLINE Buf& append(const char* p) {
	  unsigned int n = (unsigned int)strlen(p);
	  if (back_ - end_ < n) reserve(cap_ + (unsigned int)n); memcpy(end_, p, n); end_ += n; return *this;
	}
	_INLINE std::string b2s() const { return std::string(data_, end_ - data_); };
	_INLINE std::string_view b2v() const { return std::string_view(data_, end_ - data_); };
	_INLINE const char* data() const { return data_; };
	_INLINE const char* c_str() const { end_[0] = 0; return data_; };
	_INLINE char& operator[](unsigned int i) const { return data_[i]; }
	_INLINE const char operator()(unsigned int i) const { return i < end_ - data_ ? data_[i] : '\0'; };
	_INLINE char& back() { return data_[end_ - data_ - 1]; }
	_INLINE char& front() { return data_[0]; }
	_INLINE void resize(unsigned int n) {
	  if (n > cap_) { this->reserve(n); }  end_ = data_ + n;
	}
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
	void erase(unsigned int a, unsigned int b = -1);
	void ensure(unsigned int l);
	bool reserve(unsigned int l);
	Buf& insert(char*& s, const char* e, const char* f);
	Buf& assign(const char* s, const char* e);
	void swap(Buf& fs) noexcept;
	void swap(Buf&& fs) noexcept;
	Buf& replace(const char* sub, const char* to, size_t maxreplace = 0);
	Buf& strip(const char* s = " \t\r\n", char d = 'b');
	bool ends_with(const char* s) const {
	  unsigned int n = (unsigned int)strlen(s); if (n == 0) return true; return n <= this->size() && memcmp(end_ - n, s, n) == 0;
	}
	bool starts_with(const char* s) const {
	  unsigned int n = (unsigned int)strlen(s); if (n == 0) return true; return n <= this->size() && memcmp(data_, s, n) == 0;
	}
	unsigned int find_first_not_of(const char* s) const {
	  if (this->empty()) return -1; unsigned int r = (unsigned int)strspn(this->c_str(), s); return data_[r] ? r : -1;
	}
	unsigned int find_first_not_of(const char* s, size_t pos) const {
	  if (this->size() <= pos) return -1; unsigned int r = (unsigned int)strspn(this->c_str() + pos, s) + (unsigned int)pos; return data_[r] ? r : -1;
	}
	unsigned int find_first_not_of(char c, unsigned int pos = 0) const {
	  if (this->size() <= pos) return -1; char s[2] = { c, '\0' };
	  unsigned int r = (unsigned int)strspn(this->c_str() + pos, s) + pos;
	  return data_[r] ? r : -1;
	}
	Buf& operator<<(const Buf& s);
	Buf& operator<<(Buf&& s);
	Buf& operator<<(unsigned long long v);
#ifdef __linux__
	_INLINE Buf& operator<<(long long l) { std::string s = std::to_string(l); return operator<<(std::string_view(s.data(), s.size())); }
	_INLINE Buf& operator<<(int l) { std::string s = std::to_string(l); return operator<<(std::string_view(s.data(), s.size())); }
	_INLINE Buf& operator<<(long l) { std::string s = std::to_string(l); return operator<<(std::string_view(s.data(), s.size())); }
	_INLINE Buf& operator<<(unsigned long l) { std::string s = std::to_string(l); return operator<<(std::string_view(s.data(), s.size())); }
	_INLINE Buf& operator<<(unsigned int l) { std::string s = std::to_string(l); return operator<<(std::string_view(s.data(), s.size())); }
	_INLINE Buf& operator<<(short l) { std::string s = std::to_string(l); return operator<<(std::string_view(s.data(), s.size())); }
	_INLINE Buf& operator<<(unsigned short l) { std::string s = std::to_string(l); return operator<<(std::string_view(s.data(), s.size())); }
#else
	_INLINE Buf& operator<<(std::string s) {
	  if (end_ + s.size() >= back_ && !reserve((unsigned int)((cap_)+s.size()))) return *this;
	  memcpy(end_, s.data(), s.size()); end_ += s.size(); return *this;
	}
	_INLINE Buf& operator<<(long long l) { return operator<<(std::to_string(l)); }
	_INLINE Buf& operator<<(int i) { return operator<<(std::to_string(i)); }
	_INLINE Buf& operator<<(long i) { return operator<<(std::to_string(i)); }
	_INLINE Buf& operator<<(unsigned long i) { return operator<<(std::to_string(i)); }
	_INLINE Buf& operator<<(unsigned int ui) { return operator<<(std::to_string(ui)); }
	_INLINE Buf& operator<<(short a) { return operator<<(std::to_string(a)); }
	_INLINE Buf& operator<<(unsigned short ua) { return operator<<(std::to_string(ua)); }
#endif
	_INLINE Buf& operator<<(std::string_view s) {
	  if (end_ + s.size() >= back_ && !reserve((unsigned int)((cap_)+s.size()))) return *this;
	  memcpy(end_, s.data(), s.size()); end_ += s.size(); return *this;
	}
	_INLINE Buf& operator<<(const char* s) { return operator<<(std::string_view(s, strlen(s))); }
	_INLINE Buf& operator<<(char v) { end_[0] = v; ++end_; return *this; }
	_INLINE Buf& operator<<(bool b) { return operator<<(b ? RES_TURE : RES_FALSE); }
	Buf& operator<<(double d);
	Buf& operator<<(float f);
	Buf& operator=(const char* s);
	Buf& operator=(std::string&& s);
	char* data_;
	char* end_;
  private:
	char* back_;
	unsigned int cap_;
  };
}
#endif