#ifndef BUFFER_HH
#define BUFFER_HH
#include <h/common.h>
#include <lexical_cast.hh>
#pragma warning(disable:4244)
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  struct Buffer {
	Buffer();
	Buffer(Buffer&& o);
	Buffer(unsigned int capacity);
	Buffer(const char* c, unsigned int capacity);
	~Buffer();
	Buffer& operator=(Buffer&& o);
	void clear();
	_INLINE void Buffer::reset() { end_ = data_; }
	_INLINE bool Buffer::empty() { return end_ == data_; };
	_INLINE unsigned int Buffer::size() { return end_ - data_; }
	_INLINE Buffer& Buffer::append(const char c) { return (*this) << c; }
	_INLINE std::string_view Buffer::data() { return std::string_view(data_, end_ - data_); }
	_INLINE std::string Buffer::c_str() { return std::string(data_, end_ - data_); };
	std::string substr(unsigned int a, unsigned int b = -1);
	size_t find(const std::string& c);
	size_t find(const char c);
	void erase(unsigned int a, unsigned int b = -1);
	bool reserve(unsigned int l);
	Buffer& insert(char*& s, const char* e, const char* f);
	Buffer& assign(const char* s, const char* e);

	Buffer& operator<<(std::string_view s);
	Buffer& operator<<(std::string s);
	Buffer& operator<<(const char* s);
	Buffer& operator<<(char v);
	Buffer& operator<<(std::size_t v);
	Buffer& operator<<(int s);
	Buffer& operator=(std::string s);
	//template <typename I>
	//Buffer& operator<<(I v);
	char* data_;
	char* end_;
  private:
	char* back_;
	bool not_null_;
	unsigned int cap_;
  };//Buffer(1000, [&](const char* d, int s) { *this << std::string_view(d, s); })
}
#endif
