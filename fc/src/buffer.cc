#include <buffer.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  Buffer::Buffer(): buffer_(new char[0x3f]), own_buffer_(true), cursor_(buffer_), end_(buffer_ + 0x3f), cap_(0x3f) {}
  Buffer::Buffer(Buffer&& o)
	: buffer_(o.buffer_), own_buffer_(o.own_buffer_), cursor_(o.cursor_), end_(o.end_), cap_(o.cap_) {
	o.buffer_ = nullptr;
	o.own_buffer_ = false;
  }
  Buffer::Buffer(unsigned short capacity)
	: buffer_(new char[capacity]), own_buffer_(true), cursor_(buffer_), end_(buffer_ + capacity), cap_(capacity) {}
  Buffer::Buffer(const char* c, unsigned short capacity)
	: buffer_(new char[capacity]), own_buffer_(true), cursor_(buffer_), end_(buffer_ + capacity), cap_(capacity) {
	memcpy(cursor_, c, capacity); cursor_ += capacity;
  }
  Buffer::~Buffer() { if (own_buffer_) delete[] buffer_; }
  Buffer& Buffer::operator=(Buffer&& o) {
	buffer_ = o.buffer_;
	own_buffer_ = o.own_buffer_;
	cursor_ = o.cursor_;
	end_ = o.end_;
	o.buffer_ = nullptr;
	o.own_buffer_ = false;
	return *this;
  }
  void Buffer::reset() { cursor_ = buffer_; }
  void Buffer::clear() {
	delete[] buffer_; buffer_ = new char[cap_]; cursor_ = buffer_; end_ = buffer_ + cap_;
  }
  std::size_t Buffer::size() { return cursor_ - buffer_; }
  Buffer& Buffer::operator<<(std::string_view s) {
	if (cursor_ + s.size() >= end_) reserve((unsigned short)((cursor_ - buffer_) + s.size()));
	memcpy(cursor_, s.data(), s.size()); cursor_ += s.size(); return *this;
  }
  Buffer& Buffer::operator<<(const char* s) { return operator<<(std::string_view(s, strlen(s))); }
  Buffer& Buffer::operator<<(char v) {
	cursor_[0] = v; ++cursor_; return *this;
  }
  Buffer& Buffer::operator<<(std::size_t v) {
	if (v == 0) operator<<('0');
	char buffer[10], * str_start = buffer;
	for (int i = 0; i < 10; ++i) {
	  if (v > 0)
		str_start = buffer + 9 - i;
	  buffer[9 - i] = (v % 10) + '0';
	  v /= 10;
	}
	operator<<(std::string_view(str_start, buffer + 10 - str_start));
	return *this;
  }
  Buffer& Buffer::operator=(std::string s) {
	if (s.size() > cap_) s.resize(cap_);
	delete[] buffer_; buffer_ = new char[cap_]; cursor_ = buffer_;
	memcpy(cursor_, s.data(), s.size());
	cursor_ += s.size(); end_ = buffer_ + cap_;
	return *this;
  }
  template <typename I>
  Buffer& Buffer::operator<<(I v) {
	std::string b = std::move(std::lexical_cast<std::string>(v));
	return operator<<(std::string_view(b.data(), b.size()));
  }
  std::string_view Buffer::data() { return std::string_view(buffer_, cursor_ - buffer_); }
}