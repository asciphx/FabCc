#include <buffer.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace cc {
  Buffer::Buffer(): buffer_(new char[999]), own_buffer_(true), cursor_(buffer_), end_(buffer_ + 999),
	flush_([this](const char* d, int s) { *this << std::string_view(d, s); }), cap_(999) {}
  Buffer::Buffer(Buffer&& o)
	: buffer_(o.buffer_), own_buffer_(o.own_buffer_), cursor_(o.cursor_), end_(o.end_), flush_(o.flush_), cap_(o.cap_) {
	o.buffer_ = nullptr;
	o.own_buffer_ = false;
  }
  Buffer::Buffer(unsigned short capacity, std::function<void(const char*, int)> flush_)
	: buffer_(new char[capacity]), own_buffer_(true), cursor_(buffer_), end_(buffer_ + capacity), cap_(capacity),
	flush_(flush_) {}
  Buffer::Buffer(void* buffer, unsigned short capacity, std::function<void(const char*, int)> flush_)
	: buffer_((char*)buffer), own_buffer_(false), cursor_(buffer_), end_(buffer_ + capacity), cap_(capacity),
	flush_(flush_) {}
  Buffer::~Buffer() {
	if (own_buffer_)
	  delete[] buffer_;
  }
  Buffer& Buffer::operator=(Buffer&& o) {
	buffer_ = o.buffer_;
	own_buffer_ = o.own_buffer_;
	cursor_ = o.cursor_;
	end_ = o.end_;
	flush_ = o.flush_;
	o.buffer_ = nullptr;
	o.own_buffer_ = false;
	return *this;
  }
  void Buffer::reset() { cursor_ = buffer_; }
  void Buffer::clear() {
	delete[] buffer_; buffer_ = new char[cap_]; cursor_ = buffer_; end_ = buffer_ + cap_; own_buffer_ = true;
  }
  void Buffer::flush() {
	flush_(buffer_, int(size()));
	reset();
  }
  std::size_t Buffer::size() { return cursor_ - buffer_; }
  void Buffer::resize(unsigned short i) { cap_ = i; }
  Buffer& Buffer::operator<<(std::string_view s) {
	if (cursor_ + s.size() >= end_) flush();
	//assert(cursor_ + s.size() < end_);
	memcpy(cursor_, s.data(), s.size());
	cursor_ += s.size();
	return *this;
  }

  Buffer& Buffer::operator<<(const char* s) { return operator<<(std::string_view(s, strlen(s))); }
  Buffer& Buffer::operator<<(char v) {
	cursor_[0] = v;
	cursor_++;
	return *this;
  }
  Buffer& Buffer::append(const char c) { return (*this) << c; }
  Buffer& Buffer::assign(const char* s, const char* e) {
	size_t l = e - s;
	if (cursor_ + l >= end_) flush();
	for (int i = -1; ++i < l; *cursor_ = s[i], ++cursor_);
	return *this;
  }
  Buffer& Buffer::operator<<(std::size_t v) {
	if (v == 0) operator<<('0');
	char buffer[10], * str_start = buffer;
	for (int i = 0; i < 10; i++) {
	  if (v > 0)
		str_start = buffer + 9 - i;
	  buffer[9 - i] = (v % 10) + '0';
	  v /= 10;
	}
	operator<<(std::string_view(str_start, buffer + 10 - str_start));
	return *this;
  }
  template <typename I>
  Buffer& Buffer::operator<<(I v) {
	std::string b = std::move(std::lexical_cast<std::string>(v));
	return operator<<(std::string_view(b.data(), b.size()));
  }
  std::string_view Buffer::to_string_view() { return std::string_view(buffer_, cursor_ - buffer_); }
  //Buffer(1000, [&](const char* d, int s) { *this << std::string_view(d, s); })
  //flush_([this](const char* d, int s) { *this << std::string_view(d, s); })
}