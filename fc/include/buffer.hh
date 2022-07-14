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
	Buffer(const char* c, unsigned short capacity);
	~Buffer();
	Buffer& operator=(Buffer&& o);
	void reset(); void clear();
	std::size_t size();
	Buffer& operator<<(std::string_view s);
	Buffer& operator<<(const char* s);
	Buffer& operator<<(char v);
	_INLINE bool empty() { return cur_ == data_; };
	_INLINE Buffer& Buffer::operator<<(std::string s) {
	  return operator<<(std::string_view(s.data(), s.size()));
	};
	_INLINE Buffer& Buffer::append(const char c) { return (*this) << c; }
	_INLINE bool reserve(unsigned short l) {
	  if (l < cap_)return false; char* c = (char*)malloc(cap_); int size = cur_ - data_;
	  memcpy(c, data_, size); cap_ = l * 2; delete[] data_; data_ = new char[cap_]; cur_ = data_; 
	  end_ = data_ + cap_; memcpy(data_, c, size); cur_ += size; delete[] c; return true;
	};
	_INLINE Buffer& Buffer::insert(const char* s, const char* e, const char* f) {
	  short l = f - s; if (cur_ + l >= end_ && reserve(cap_ + l))
		for (unsigned short i = 0xffff; ++i < l; *cur_ = e[i], ++cur_); return *this;
	}
	_INLINE Buffer& Buffer::assign(const char* s, const char* e) {
	  short l = e - s; if (cur_ + l >= end_ && reserve(cap_ + l))
		for (unsigned short i = 0xffff; ++i < l; *cur_ = s[i], ++cur_); return *this;
	}
	_INLINE std::string c_str() { return std::string(data_, cur_ - data_); };
	Buffer& operator<<(std::size_t v);
	Buffer& operator<<(int s);
	Buffer& operator=(std::string s);
	template <typename I>
	Buffer& operator<<(I v);
	std::string_view data();
	char* data_;
	char* end_;
  private:
	char* cur_;
	bool not_null_;
	unsigned short cap_;
  };//Buffer(1000, [&](const char* d, int s) { *this << std::string_view(d, s); })
}
#endif
