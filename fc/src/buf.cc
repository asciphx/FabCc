#include <buf.hh>
#include <utility>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/output_buffer.hh
namespace fc {
  Buf::Buf(): data_(new char[0x3f]), end_(data_), back_(data_ + 0x3f), cap_(0x3f), not_null_(true) {}
  Buf::Buf(Buf&& o): data_(o.data_), end_(o.end_), back_(o.back_), cap_(o.cap_), not_null_(o.not_null_) {
	o.data_ = nullptr; o.not_null_ = false; o.cap_ = 0;
  }
  Buf::Buf(const Buf& o): data_(new char[o.cap_]), end_(data_), back_(data_ + o.cap_), cap_(o.cap_), not_null_(true) {
	memcpy(end_, o.data_, o.end_ - o.data_); end_ += o.end_ - o.data_;
  }
  Buf::Buf(unsigned int capacity)
	: data_(new char[capacity]), end_(data_), back_(data_ + capacity), cap_(capacity), not_null_(true) {}
  Buf::Buf(const char* c, unsigned int capacity)
	: data_(new char[capacity]), end_(data_), back_(data_ + capacity), cap_(capacity), not_null_(true) {
	memcpy(end_, c, capacity); end_ += capacity;
  }
  Buf::Buf(const char* c): cap_((unsigned int)strlen(c)),
	data_(new char[cap_]), end_(data_), back_(data_ + cap_), not_null_(true) {
	memcpy(end_, c, cap_); end_ += cap_;
  };
  Buf::Buf(const std::string& s):cap_((unsigned int)s.size()),
	data_(new char[cap_]), end_(data_), back_(data_ + cap_), not_null_(true) {
	memcpy(end_, s.c_str(), cap_); end_ += cap_;
  };
  Buf::Buf(size_t n, char c): cap_((unsigned int)n),
	data_(new char[cap_]), end_(data_), back_(data_ + cap_), not_null_(true) {
	memset(end_, c, n); end_ += n;
  }
  Buf::~Buf() { if (not_null_) delete[] data_; }
  Buf& Buf::operator=(Buf&& o) {
	std::swap(o.back_, back_); std::swap(o.cap_, cap_); std::swap(o.end_, end_); std::swap(o.data_, data_); return *this;
  }
  Buf& Buf::operator=(const Buf& o) {
	if (&o != this) {
	  if (cap_ < o.cap_) {
		delete[] data_; cap_ = o.cap_; data_ = new char[cap_]; back_ = data_ + cap_;
	  } end_ = data_;
	  memcpy(end_, o.data_, o.end_ - o.data_); end_ += o.end_ - o.data_;
	}
	return *this;
  }
  void Buf::clear() { if (not_null_) delete[] data_; data_ = new char[cap_]; end_ = data_; back_ = data_ + cap_; }
  Buf Buf::substr(unsigned int a) const {
	unsigned int l = end_ - data_; return Buf(a > l ? data_ : data_ + a, l - a);
  }
  Buf Buf::substr(unsigned int a, unsigned int b) const {
	unsigned int l = end_ - data_; return Buf(a > l ? data_ : data_ + a, a + b < l ? b : l - b);
  }
  unsigned int Buf::find(const char* c) const {
	unsigned int l = 0, L = (unsigned int)strlen(c), s = end_ - data_, a = 0; while (l < s) {
	  if (data_[l] != c[a])a = 0; ++l; ++a; if (a == L) { return l - L; }
	} return -1;
  }
  unsigned int Buf::find(const std::string& c) const {
	unsigned int l = 0, L = (unsigned int)c.size(), s = end_ - data_, a = 0; while (l < s) {
	  if (data_[l] != c[a])a = 0; ++l; ++a; if (a == L) { return l - L; }
	} return -1;
  }
  unsigned int Buf::find(const char c) const {
	unsigned int l = 0, s = end_ - data_; while (l < s) { if (data_[l] == c) { return l; } ++l; } return -1;
  }
  unsigned int Buf::find(const char* s, size_t pos) const {
	if (this->size() <= pos) return -1; const char* p = strstr(this->c_str() + pos, s); return p ? p - data_ : -1;
  }
  unsigned int Buf::find(char c, size_t pos) const {
	if (this->size() <= pos) return -1; char* p = (char*)memchr(data_ + pos, c, end_ - data_ - pos); return p ? p - data_ : -1;
  }
  unsigned int Buf::rfind(const char c) const {
	unsigned int s = end_ - data_; while (0 < s) { if (data_[s] == c) { return s; } --s; } return -1;
  }
  Buf& Buf::append(const Buf& s) {
	if (&s != this) {
	  if (s.empty()) return *this;
	  return *this << s;
	} else { /* append itself */
	  if (data_) {
		this->reserve(this->size() + 10);
		memcpy(end_, data_, end_ - data_);
		end_ += end_ - data_;
	  }
	  return *this;
	}
  }
  unsigned int Buf::rfind(const char* sub) const {
	unsigned int m = (unsigned int)strlen(sub);
	if (m == 1) return this->rfind(*sub);
	unsigned int n = this->size();
	if (n < m) return -1;
	unsigned int tbl[256] = { 0 };
	for (unsigned int i = m; i > 0; --i) tbl[sub[i - 1]] = i;
	for (unsigned int j = n - m;;) {
	  if (memcmp(sub, data_ + j, m) == 0) return j;
	  if (j == 0) return -1;
	  unsigned int x = tbl[data_[j - 1]];
	  if (x == 0) x = m + 1;
	  if (j < x) return -1;
	  j -= x;
	}
  }
  void Buf::erase(unsigned int a, unsigned int b) {
	unsigned int l = end_ - data_; if (a > l)a = l; if (b > l)b = l;
	char* c = (char*)malloc(cap_); memcpy(c, data_, a); memcpy(c, data_ + b, l - b);
	delete[] data_; data_ = new char[cap_]; end_ = data_; b = a + l - b;
	back_ = data_ + cap_; memcpy(data_, c, b); end_ += b; delete[] c;
  }
  bool Buf::ensure(unsigned int l) {
	char* c = (char*)malloc(cap_); unsigned int size = end_ - data_; memcpy(c, data_, size);
	cap_ += (cap_ >> 1) + l; delete[] data_; data_ = new char[cap_]; end_ = data_;
	back_ = data_ + cap_; memcpy(data_, c, size); end_ += size; delete[] c; return true;
  };
  bool Buf::reserve(unsigned int l) {
	if (l < cap_)return false; char* c = (char*)malloc(cap_); unsigned int size = end_ - data_;
	memcpy(c, data_, size); cap_ += cap_; cap_ += l; delete[] data_; data_ = new char[cap_]; end_ = data_;
	back_ = data_ + cap_; memcpy(data_, c, size); end_ += size; delete[] c; return true;
  };
  Buf& Buf::insert(char*& s, const char* e, const char* f) {
	unsigned int l = f - e; if (s + l >= back_ && !reserve(cap_ + l)) return *this; memcpy(s, e, l); s += l; return *this;
  }
  Buf& Buf::assign(const char* s, const char* e) {
	unsigned int l = e - s; if (end_ + l >= back_ && !reserve(cap_ + l)) return *this; memcpy(end_, s, l); end_ += l; return *this;
  }
  void Buf::swap(Buf& b) noexcept {
	std::swap(b.cap_, cap_); std::swap(b.back_, back_); std::swap(b.end_, end_); std::swap(b.data_, data_);
  }
  void Buf::swap(Buf&& b) noexcept { b.swap(*this); }
  Buf& Buf::replace(const char* sub, const char* to, size_t maxreplace) {
	if (this->empty()) return *this;
	const char* from = this->c_str();
	const char* p = strstr(from, sub);
	if (!p) return *this;
	size_t n = strlen(sub);
	size_t m = strlen(to);
	Buf s(end_ - data_);
	do {
	  s.append(from, p - from).append(to, m);
	  from = p + n;
	  if (maxreplace && --maxreplace == 0) break;
	} while ((p = strstr(from, sub)));
	if (from < end_) s.append(from);
	this->swap(s);
	return *this;
  }
  Buf& Buf::strip(const char* s, char d) {
	if (this->empty()) return *this;
	char bs[0x100] = { 0 };
	while (*s) bs[(const unsigned char)(*s++)] = 1;
	unsigned int e = 0;
	if (d == 'l' || d == 'L') {
	  while (e < end_ - data_ && bs[(unsigned char)(data_[e])]) ++e;
	  if (e != 0 && (end_ -= e) != 0) memmove(data_, data_ + e, size());
	} else if (d == 'r' || d == 'R') {
	  unsigned int _s = e = end_ - data_;
	  while (e > 0 && bs[(unsigned char)(data_[e - 1])]) --e;
	  if (e != _s) end_ = data_ + e;
	} else {
	  unsigned int _s = e = end_ - data_;
	  while (e > 0 && bs[(unsigned char)(data_[e - 1])]) --e;
	  if (e != _s) end_ = data_ + e;
	  if (e == 0) return *this;
	  e = 0;
	  while (e < _s && bs[(unsigned char)(data_[e])]) ++e;
	  if (e != 0 && (_s -= e) != 0) memmove(data_, data_ + e, size());
	}
	return *this;
  }
  Buf& Buf::operator<<(const Buf& buf) {
	const Buf* b = &buf; unsigned int l = b->end_ - b->data_;
	if (end_ + l >= back_ && !reserve((unsigned int)((end_ - data_) + l))) return *this;
	memcpy(end_, b->data_, l); end_ += l; return *this;
  }
  Buf& Buf::operator<<(unsigned long long v) {
	if (v == 0) operator<<('0'); char mega_buffer[20], * str_start = mega_buffer; for (int i = 0; v != 0; ++i) {
	  str_start = mega_buffer + 19 - i; mega_buffer[19 - i] = (v % 10) + '0'; v /= 10;
	} return *this << (std::string_view(str_start, mega_buffer + 20 - str_start));
  }
  Buf& Buf::operator=(const char* s) {
	unsigned int l = (unsigned int)strlen(s); if (l > cap_ && !reserve(cap_ + l)) return *this;
	delete[] data_; data_ = new char[cap_]; end_ = data_; back_ = data_ + cap_; return *this << (std::string_view(s, l));
  }
  Buf& Buf::operator=(std::string&& s) {
	if (s.size() > cap_ && !reserve(cap_ + (unsigned int)s.size())) return *this;
	delete[] data_; data_ = new char[cap_]; end_ = data_; back_ = data_ + cap_;
	return *this << std::string_view(s.data(), s.size());
  }
}