#include <buffer.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  Buffer::Buffer(): data_(new char[0x3f]), not_null_(true), end_(data_), back_(data_ + 0x3f), cap_(0x3f) {}
  Buffer::Buffer(Buffer&& o): data_(o.data_), not_null_(o.not_null_), end_(o.end_), back_(o.back_), cap_(o.cap_) {
	o.data_ = nullptr; o.not_null_ = false;
  }
  Buffer::Buffer(const Buffer& o): data_(new char[o.cap_]), not_null_(true), end_(data_), back_(data_ + o.cap_), cap_(o.cap_) {
	memcpy(end_, o.data_, o.end_ - o.data_); end_ +=  o.end_ - o.data_;
  }
  Buffer::Buffer(unsigned int capacity)
	: data_(new char[capacity]), not_null_(true), end_(data_), back_(data_ + capacity), cap_(capacity) {}
  Buffer::Buffer(const char* c, unsigned int capacity)
	: data_(new char[capacity]), not_null_(true), end_(data_), back_(data_ + capacity), cap_(capacity) {
	memcpy(end_, c, capacity); end_ += capacity;
  }
  Buffer::~Buffer() { if (not_null_) delete[] data_; }
  Buffer& Buffer::operator=(Buffer&& o) {
	data_ = o.data_; not_null_ = o.not_null_; end_ = o.end_; back_ = o.back_; cap_ = o.cap_;
	o.data_ = nullptr; o.not_null_ = false; return *this;
  }
  void Buffer::clear() { delete[] data_; data_ = new char[cap_]; end_ = data_; back_ = data_ + cap_; }
  std::string Buffer::substr(unsigned int a, unsigned int b) {
	return std::string(data_ + a, b < end_ - data_ ? b : end_ - data_);
  }
  size_t Buffer::find(const std::string& c) {
	size_t l = 0, L = c.size(), a = 0; while (data_[l]) {
	  if (data_[l] != c[a])a = 0; ++l; ++a; if (a == L) { return l - L; }
	} return -1;
  }
  size_t Buffer::find(const char c) {
	size_t l = 0; while (data_[l]) { if (data_[l] == c) { return l; } ++l; } return -1;
  }
  void Buffer::erase(unsigned int a, unsigned int b) {
	unsigned int l = end_ - data_; if (a > l)a = l; if (b > l)b = l;
	char* c = (char*)malloc(cap_); memcpy(c, data_, a); memcpy(c, data_ + b, l - b);
	delete[] data_; data_ = new char[cap_]; end_ = data_; b = a + l - b;
	back_ = data_ + cap_; memcpy(data_, c, b); end_ += b; delete[] c;
  }
  bool Buffer::reserve(unsigned int l) {
	if (l < cap_)return false; char* c = (char*)malloc(cap_); int size = end_ - data_;
	memcpy(c, data_, size); cap_ = l * 2; delete[] data_; data_ = new char[cap_]; end_ = data_;
	back_ = data_ + cap_; memcpy(data_, c, size); end_ += size; delete[] c; return true;
  };
  Buffer& Buffer::insert(char*& s, const char* e, const char* f) {
	unsigned int l = f - e; if (s + l >= back_ && !reserve(cap_ + l)) return *this;
	memcpy(s, e, l); s += l; return *this;
  }
  Buffer& Buffer::assign(const char* s, const char* e) {
	unsigned int l = e - s; if (end_ + l >= back_ && !reserve(cap_ + l)) return *this;
	for (unsigned int i = 0xffffffff; ++i < l; *end_ = s[i], ++end_); return *this;
  }
  Buffer& Buffer::operator<<(std::string_view s) {
	if (end_ + s.size() >= back_) reserve((unsigned int)((end_ - data_) + s.size()));
	memcpy(end_, s.data(), s.size()); end_ += s.size(); return *this;
  }
  Buffer& Buffer::operator<<(const char* s) { return operator<<(std::string_view(s, strlen(s))); }
  Buffer& Buffer::operator<<(char v) { end_[0] = v; ++end_; return *this; }
  Buffer& Buffer::operator<<(std::size_t v) {
	if (v == 0) operator<<('0'); char mega_buffer[10], * str_start = mega_buffer;
	for (char i = 0; i < 10; ++i) {
	  if (v > 0) str_start = mega_buffer + 9 - i; mega_buffer[9 - i] = (v % 10) + '0'; v /= 10;
	}
	operator<<(std::string_view(str_start, mega_buffer + 10 - str_start)); return *this;
  }
  Buffer& Buffer::operator<<(int b) {
	std::string s(std::lexical_cast<std::string>(b)); return operator<<(std::string_view(s.data(), s.size()));
  }
  Buffer& Buffer::operator=(std::string s) {
	if (s.size() > cap_) s.resize(cap_); delete[] data_; data_ = new char[cap_]; end_ = data_;
	memcpy(end_, s.data(), s.size()); end_ += s.size(); back_ = data_ + cap_; return *this;
  }
  // template <typename I>
  // Buffer& Buffer::operator<<(I v) {
	 //std::string b(std::lexical_cast<std::string>(v));return operator<<(std::string_view(b.data(), b.size()));
  // }
}