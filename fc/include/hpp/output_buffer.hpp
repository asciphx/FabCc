#ifndef OUT_BUF_HPP
#define OUT_BUF_HPP
#include <functional>
#include <tp/c++.h>
#include <hpp/string_view.hpp>
#include <hh/lexical_cast.hh>
#include <hh/conn.hh>
#define _OPT(z, x) _FORCE_INLINE output_buffer& z(x s) { size_t S = s.size(); if (this->cursor_ + S > end_) {\
size_t G = this->cap_ - this->size(); memcpy(cursor_, s.data(), G); this->cursor_ += G; S -= G; this->flush();\
bool b = this->cap_ < S; _:if (b) { memcpy(this->cursor_, s.data() + G, this->cap_); this->cursor_ += this->cap_;\
G += this->cap_; this->flush(); S -= this->cap_; b = this->cap_ < S; goto _; } memcpy(this->cursor_, s.data() + G, S);\
this->cursor_ += S; return *this;} memcpy(this->cursor_, s.data(), S); this->cursor_ += S; return *this; }
namespace fc {
  struct output_buffer {
    output_buffer(): buffer_(nullptr), cursor_(nullptr), end_(nullptr), cap_(0), flush_(nullptr) {}
    output_buffer(char* buffer, size_t capacity, Conn* flush_ = nullptr)
      : buffer_(buffer), cursor_(buffer_), end_(buffer_ + capacity), flush_(flush_), cap_(capacity) {}
    output_buffer& operator=(output_buffer&& o) {
      buffer_ = o.buffer_; cursor_ = o.cursor_; end_ = o.end_; cap_ = o.cap_; flush_ = o.flush_; o.buffer_ = nullptr; return *this;
    }
    _FORCE_INLINE void reset() { cursor_ = buffer_; }
    _FORCE_INLINE std::size_t size() { return cursor_ - buffer_; }
#if __cplusplus < _cpp20_date
    _FORCE_INLINE
#endif
    _CTX_TASK(int) flush() { co_await flush_->write(buffer_, int(size())); reset(); _CTX_return(1) }
    _OPT(operator<<, const std::string_view&)_OPT(operator<<, std::string_view&&)_OPT(operator<<, std::string&)_OPT(append, std::string_view&);
    _FORCE_INLINE output_buffer& append(const char* s, size_t size) {
      if (cursor_ + size > end_) flush(); memcpy(cursor_, s, size); cursor_ += size; return *this;
    }
    _FORCE_INLINE output_buffer& operator<<(unsigned int v) {
      if (v == 0) operator<<('0'); char c[10]; return operator<<(std::string_view(c, u2a(c, v) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(char v) { cursor_[0] = v; ++cursor_; return *this; }
    _FORCE_INLINE output_buffer& operator<<(unsigned long long& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, u64toa(c, v) - c));
    }
#ifdef _WIN32
    _FORCE_INLINE output_buffer& operator<<(long long& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, i64toa(c, v) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(long& v) {
      if (v == 0) operator<<('0'); char c[11]; return operator<<(std::string_view(c, i2a(c, v) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(long&& v) {
      if (v == 0) operator<<('0'); char c[11]; return operator<<(std::string_view(c, i2a(c, std::move(v)) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(unsigned long& v) {
      if (v == 0) operator<<('0'); char c[10]; return operator<<(std::string_view(c, u2a(c, v) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(unsigned long&& v) {
      if (v == 0) operator<<('0'); char c[10]; return operator<<(std::string_view(c, u2a(c, std::move(v)) - c));
    }
#else
    _FORCE_INLINE output_buffer& operator<<(long long& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, i64toa(c, v) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(long& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, i64toa(c, v) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(long&& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, i64toa(c, std::move(v)) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(unsigned long& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, u64toa(c, v) - c));
    }
    _FORCE_INLINE output_buffer& operator<<(unsigned long&& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, u64toa(c, std::move(v)) - c));
    }
#endif // _WIN32
    _FORCE_INLINE output_buffer& operator<<(unsigned long long&& v) {
      if (v == 0) operator<<('0'); char c[20]; return operator<<(std::string_view(c, u64toa(c, std::move(v)) - c));
    }
    _FORCE_INLINE output_buffer& append(const char c) { return (*this) << c; }
    template <typename I, std::enable_if_t<std::is_fundamental<I>::value>* = nullptr>
    _FORCE_INLINE output_buffer& operator<<(std::remove_reference_t<I>&& v) {
      std::string s(std::move(std::lexical_cast<std::string>(std::forward<I>(v))));
      if (cursor_ + s.size() > end_) flush(); memcpy(cursor_, s.data(), s.size()); cursor_ += s.size(); return *this;
    }
    _FORCE_INLINE std::string_view to_string_view() { return std::string_view(buffer_, cursor_ - buffer_); }
    Conn* flush_; char* buffer_, * cursor_, * end_; size_t cap_;
  };
} // namespace fc
#endif