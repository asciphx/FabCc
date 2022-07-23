#ifndef BUFFER_HH
#define BUFFER_HH
#include <h/common.h>
#include <lexical_cast.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  struct Buffer {
	Buffer();
	Buffer(Buffer&& o);
	Buffer(unsigned int capacity);
	Buffer(const char* c, unsigned int capacity);
	~Buffer();
	Buffer& operator=(Buffer&& o);
	_INLINE void reset(); void clear();
	bool empty();
	Buffer& operator<<(std::string s);
	Buffer& append(const char c);
	_INLINE bool reserve(unsigned int l);
	Buffer& insert(const char* s, const char* e, const char* f);
	Buffer& assign(const char* s, const char* e);
	std::string c_str();
	std::size_t size();
	Buffer& operator<<(std::string_view s);
	Buffer& operator<<(const char* s);
	Buffer& operator<<(char v);
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
	unsigned int cap_;
  };//Buffer(1000, [&](const char* d, int s) { *this << std::string_view(d, s); })
}
#endif
