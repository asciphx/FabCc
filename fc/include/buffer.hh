#ifndef BUFFER_HH
#define BUFFER_HH
#include <h/common.h>
#include <lexical_cast.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  struct Buffer {
	Buffer();
	Buffer(Buffer&& o);
	Buffer(unsigned short capacity);
	~Buffer();
	Buffer& operator=(Buffer&& o);
	void reset(); void clear();
	std::size_t size();
	Buffer& operator<<(std::string_view s);
	Buffer& operator<<(const char* s);
	Buffer& operator<<(char v);
	Buffer& operator=(std::string s);
	_INLINE bool empty() { return cursor_ == buffer_; };
	_INLINE Buffer& Buffer::operator<<(std::string s) {
	  return operator<<(std::string_view(s.data(), s.size()));
	};
	_INLINE Buffer& Buffer::append(const char c) { return (*this) << c; }
	_INLINE void reserve(unsigned short l) {
	  if (l < cap_)return;
	  char* c = (char*)malloc(cap_); int size = cursor_ - buffer_;
	  memcpy(c, buffer_, size); cap_ = l * 2;
	  delete[] buffer_; buffer_ = new char[cap_]; cursor_ = buffer_; end_ = buffer_ + cap_;
	  memcpy(buffer_, c, size); cursor_ += size; delete[] c;
	};
	_INLINE Buffer& Buffer::insert(const char* s, const char* e, const char* f) {
	  short l = f - s;
	  if (cursor_ + l >= end_) reserve(cap_ + l);
	  for (unsigned short i = 0xffff; ++i < l; *cursor_ = e[i], ++cursor_);
	  return *this;
	}
	_INLINE Buffer& Buffer::assign(const char* s, const char* e) {
	  short l = e - s;
	  if (cursor_ + l >= end_) reserve(cap_ + l);
	  for (unsigned short i = 0xffff; ++i < l; *cursor_ = s[i], ++cursor_);
	  return *this;
	}
	Buffer& operator<<(std::size_t v);
	// template <typename I>
	// Buffer& operator<<(unsigned long s)
	// {
	//   typedef std::array<char, 150> buf_t;
	//   buf_t b = std::lexical_cast<buf_t>(v);
	//   return operator<<(std::string_view(b.begin(), strlen(b.begin())));
	// }
	template <typename I>
	Buffer& operator<<(I v);
	std::string_view data();
	char* buffer_;
	char* cursor_;
	char* end_;
  private:
	bool own_buffer_;
	unsigned short cap_;
  };//Buffer(1000, [&](const char* d, int s) { *this << std::string_view(d, s); })
}
#endif
