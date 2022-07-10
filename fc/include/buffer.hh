#ifndef BUFFER_HH
#define BUFFER_HH
#include <functional>
#include <h/common.h>
#include <lexical_cast.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  struct Buffer {
	Buffer();
	Buffer(Buffer&& o);
	Buffer(unsigned short capacity, std::function<void(const char*, int)> flush_ = [](const char*, int) {});
	Buffer(void* buffer, unsigned short capacity,
		std::function<void(const char*, int)> flush_ = [](const char*, int) {});
	~Buffer();
	Buffer& operator=(Buffer&& o);
	void reset(); void clear(); void flush();
	std::size_t size();
	void reserve(unsigned short i);
	Buffer& operator<<(std::string_view s);
	Buffer& operator<<(const char* s);
	Buffer& operator<<(char v);
	_INLINE Buffer& Buffer::operator<<(std::string s) {
	  return operator<<(std::string_view(s.data(), s.size()));
	};
	_INLINE Buffer& Buffer::append(const char c) { return (*this) << c; }
	_INLINE Buffer& Buffer::assign(const char* s, const char* e) {
	  size_t l = e - s;
	  if (cursor_ + l >= end_) flush();
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
	std::string_view to_string_view();
	char* buffer_;
	char* cursor_;
	std::function<void(const char* s, int d)> flush_;
  private:
	char* end_;
	bool own_buffer_;
	unsigned short cap_;
  };//Buffer(1000, [&](const char* d, int s) { *this << std::string_view(d, s); })
}
#endif
