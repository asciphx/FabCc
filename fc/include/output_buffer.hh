#pragma once
#include <functional>
#include <tp/c++.h>
#include <hpp/string_view.hpp>
#include <lexical_cast.hh>
namespace fc {
  struct output_buffer {
    output_buffer(): buffer_(nullptr), own_buffer_(false), cursor_(nullptr), end_(nullptr),
      flush_([](const char*, int) __CONSTEXPR{}) {}
    output_buffer(output_buffer&& o)
      : buffer_(o.buffer_), own_buffer_(o.own_buffer_), cursor_(o.cursor_), end_(o.end_),
      flush_(o.flush_) {
      o.buffer_ = nullptr;
      o.own_buffer_ = false;
    }
    output_buffer(
      int capacity, std::function<void(const char*, int)> flush_ = [](const char*, int) {})
      : buffer_(new char[capacity]), own_buffer_(true), cursor_(buffer_), end_(buffer_ + capacity),
      flush_(flush_) {}
    output_buffer(
      void* buffer, int capacity,
      std::function<void(const char*, int)> flush_ = [](const char*, int) {})
      : buffer_((char*)buffer), own_buffer_(false), cursor_(buffer_), end_(buffer_ + capacity),
      flush_(flush_) {}
    ~output_buffer() {
      if (own_buffer_) delete[] buffer_;
    }
    output_buffer& operator=(output_buffer&& o) {
      buffer_ = o.buffer_;
      own_buffer_ = o.own_buffer_;
      cursor_ = o.cursor_;
      end_ = o.end_;
      flush_ = o.flush_;
      o.buffer_ = nullptr;
      o.own_buffer_ = false;
      return *this;
    }
    void reset() { cursor_ = buffer_; }
    std::size_t size() { return cursor_ - buffer_; }
    void flush() {
      flush_(buffer_, int(size()));
      reset();
    }
    output_buffer& operator<<(std::string_view s) {
      if (cursor_ + s.size() >= end_) flush();
      assert(cursor_ + s.size() < end_);
      memcpy(cursor_, s.data(), s.size());
      cursor_ += s.size();
      return *this;
    }
    output_buffer& operator<<(std::string& s) {
      if (cursor_ + s.size() >= end_) flush();
      assert(cursor_ + s.size() < end_);
      memcpy(cursor_, s.data(), s.size());
      cursor_ += s.size();
      return *this;
    }
    output_buffer& operator<<(const char* s) { return operator<<(std::string_view(s, strlen(s))); }
    output_buffer& operator<<(char v) {
      cursor_[0] = v; ++cursor_; return *this;
    }
    output_buffer& operator<<(size_t& v) {
      if (v == 0) operator<<('0'); char c[20];
      return operator<<(std::string_view(c, jeaiii::to_text_from_integer(c, v) - c));
    }
    output_buffer& operator<<(size_t&& v) {
      if (v == 0) operator<<('0'); char c[20];
      return operator<<(std::string_view(c, jeaiii::to_text_from_integer(c, std::move(v)) - c));
    }
    inline output_buffer& append(const char c) { return (*this) << c; }
    template <typename I, std::enable_if_t<std::is_fundamental<I>::value>* = nullptr>
    output_buffer& operator<<(I& v) {
      return operator<<(std::lexical_cast<std::string>(v));
    }
    template <typename I, std::enable_if_t<std::is_fundamental<I>::value>* = nullptr>
    output_buffer& operator<<(I&& v) {
      return operator<<(std::lexical_cast<std::string>(std::move(v)));
    }
    std::string_view to_string_view() { return std::string_view(buffer_, cursor_ - buffer_); }
    char* buffer_;
    bool own_buffer_;
    char* cursor_;
    char* end_;
    std::function<void(const char* s, int d)> flush_;
  };
} // namespace fc
