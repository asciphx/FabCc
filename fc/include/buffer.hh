#ifndef BUFFER_HH
#define BUFFER_HH
#include <cstring>
#include <string>
#pragma warning(disable:4244)
#if defined(_MSC_VER) && !defined(_INLINE)
#define _INLINE __forceinline
#elif !defined(_INLINE)
#define _INLINE inline
#endif
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  struct Buffer {
	Buffer();
	Buffer(Buffer&& o);
	Buffer(const Buffer& o);
	Buffer(unsigned int capacity);
	Buffer(const char* c, unsigned int capacity);
	//Buffer(const char* c);
	~Buffer();
	Buffer& operator=(Buffer&& o);
	Buffer& operator=(Buffer& o);
	void clear();
	
	_INLINE void push_back(const char c) { end_[0] = c; ++end_; }
	_INLINE void reset() { end_ = data_; }
	_INLINE bool empty() { return end_ == data_; };
	_INLINE unsigned int size() const { return end_ - data_; }
	_INLINE unsigned int length() const { return end_ - data_; }
	_INLINE Buffer& append(const char c) { return (*this) << c; }
	_INLINE std::string b2s() { return std::string(data_, end_ - data_); };
	_INLINE std::string_view b2v() const { return std::string_view(data_, end_ - data_); };
	_INLINE const char* data() const { end_[0] = 0; return data_; };
	_INLINE const char* c_str() const { end_[0] = 0; return data_; };
	_INLINE char operator[](unsigned int i) { return i < end_ - data_ ? data_[i] : '\0'; };
	_INLINE const char back() { return data_[end_ - data_ - 1]; }
	
	std::string_view substr(unsigned int a);
	std::string_view substr(unsigned int a, unsigned int b);
	unsigned int find(const char* c);
	unsigned int find(const std::string& c);
	unsigned int find(const char c);
	unsigned int rfind(const char c);
	void erase(unsigned int a, unsigned int b = -1);
	bool reserve(unsigned int l);
	Buffer& insert(char*& s, const char* e, const char* f);
	Buffer& assign(const char* s, const char* e);
	
	Buffer& operator<<(const Buffer& s);
	Buffer& operator<<(size_t v);
	//Buffer& operator<<(std::string_view s);
	_INLINE Buffer& operator<<(std::string_view s) {
	  if (end_ + s.size() >= back_ && !reserve((unsigned int)((cap_) + s.size()))) return *this;
	  memcpy(end_, s.data(), s.size()); end_ += s.size(); return *this;
	}
	_INLINE Buffer& operator<<(const char* s) { return operator<<(std::string_view(s, strlen(s))); }
	_INLINE Buffer& operator<<(char v) { end_[0] = v; ++end_; return *this; }
	_INLINE Buffer& operator<<(long long l) { return operator<<(std::to_string(l)); }
	_INLINE Buffer& operator<<(int i) { return operator<<(std::to_string(i)); }
	_INLINE Buffer& operator<<(unsigned int ui) { return operator<<(std::to_string(ui)); }
	Buffer& operator=(const char* s);
	Buffer& operator=(std::string_view s);
	char* data_;
	char* end_;
  private:
	char* back_;
	bool not_null_;
	unsigned int cap_;
  };
}
#endif
