#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP
#if (defined(_HAS_CXX17) && _HAS_CXX17 == 1) || (defined(__cplusplus) && __cplusplus >= 201703L)
#include <string_view>
#else//__cplusplus <= 201402L
#include <stdexcept>
#include <string>
#include <ostream>
#include <algorithm>
#include <memory>
#include <iterator>
#include <exception>
#include <istream>
namespace std {
  class string_view {
	const char* data_; size_t length_;
  public:
	static constexpr size_t npos = size_t(-1);
	constexpr string_view() noexcept: data_(nullptr), length_(0) {}
	constexpr string_view(const char* data, size_t length) : data_(data), length_(length) {}
	string_view(const char* data): data_(data), length_(char_traits<char>::length(data)) {}
	constexpr string_view(const string_view& other) noexcept = default;
	string_view(const basic_string<char, char_traits<char>, allocator<char>>& s) noexcept
	  : data_(s.data()), length_(s.length()) {}
	string_view& operator=(const string_view& view) noexcept {
	  data_ = view.data(); length_ = view.length(); return *this;
	}
	explicit operator basic_string<char, char_traits<char>, allocator<char>>() const {
	  return basic_string<char, char_traits<char>, allocator<char>>(data_, length_);
	}
	const char* begin() const noexcept { return data_; }
	const char* end() const noexcept { return data_ + length_; }
	size_t size() const { return length_; }
	size_t length() const { return length_; }
	bool empty() const noexcept { return length_ == 0; }
	const char& operator[](size_t pos) const { return data_[pos]; }
	const char& at(size_t pos) const {
	  if (pos >= length_) { throw out_of_range("pos exceeds length"); } return data_[pos];
	}
	const char& front() const { return data_[0]; }
	const char& back()  const { return data_[length_ - 1]; }
	const char* data() const { return data_; }
	int compare(const string_view& s) const noexcept {
	  const int rc = char_traits<char>::compare(data_, s.data_, (min)(length_, s.length_));
	  return rc != 0 ? rc : (length_ == s.length_ ? 0 : length_ < s.length_ ? -1 : 1);
	}
	int compare(const char* data) const noexcept {
	  const size_t l = char_traits<char>::length(data);
	  const int rc = char_traits<char>::compare(data_, data, (min)(length_, l));
	  return rc != 0 ? rc : (length_ == l ? 0 : length_ < l ? -1 : 1);
	}
	int compare(const basic_string<char, char_traits<char>, allocator<char>>& s) const noexcept {
	  const int rc = char_traits<char>::compare(data_, s.data(), (min)(length_, s.length()));
	  return rc != 0 ? rc : (length_ == s.length() ? 0 : length_ < s.length() ? -1 : 1);
	}
	friend basic_ostream<char>& operator<<(basic_ostream<char>& os, const string_view& sv) {
	  os.write(sv.data_, sv.length_); return os;
	}
	friend string& operator+=(string& s, const string_view& _v) { s.append(_v.data_, _v.length_); return s; };
  };
  // ==
  inline bool operator==(const string_view& l, const string_view& r) noexcept {
	return l.compare(r) == 0;
  }
  inline bool operator==(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
	return l.compare(r) == 0;
  }
  inline bool operator==(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
	return r.compare(l) == 0;
  }
  inline bool operator==(const string_view& l, const char* r) noexcept {
	return l.compare(r) == 0;
  }
  inline bool operator==(const char* l, const string_view& r) noexcept {
	return r.compare(l) == 0;
  }
  // !=
  inline bool operator!=(const string_view& l, const string_view& r) noexcept {
	return l.compare(r) != 0;
  }
  inline bool operator!=(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
	return l.compare(r) != 0;
  }
  inline bool operator!=(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
	return r.compare(l) != 0;
  }
  inline bool operator!=(const string_view& l, const char* r) noexcept {
	return l.compare(r) != 0;
  }
  inline bool operator!=(const char* l, const string_view& r) noexcept {
	return r.compare(l) != 0;
  }
  // <=
  inline bool operator<=(const string_view& l, const string_view& r) noexcept {
	return l.compare(r) <= 0;
  }
  inline bool operator<=(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
	return l.compare(r) <= 0;
  }
  inline bool operator<=(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
	return r.compare(l) >= 0;
  }
  // <
  inline bool operator<(const string_view& l, const string_view& r) noexcept {
	return l.compare(r) < 0;
  }
  inline bool operator<(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
	return l.compare(r) < 0;
  }
  inline bool operator<(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
	return r.compare(l) > 0;
  }
  // >=
  inline bool operator>=(const string_view& l, const string_view& r) noexcept {
	return l.compare(r) >= 0;
  }
  inline bool operator>=(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
	return l.compare(r) >= 0;
  }
  inline bool operator>=(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
	return r.compare(l) <= 0;
  }
  // >
  inline bool operator>(const string_view& l, const string_view& r) noexcept {
	return l.compare(r) > 0;
  }
  inline bool operator>(const string_view& l, const basic_string<char, char_traits<char>, allocator<char>>& r) noexcept {
	return l.compare(r) > 0;
  }
  inline bool operator>(const basic_string<char, char_traits<char>, allocator<char>>& l, const string_view& r) noexcept {
	return r.compare(l) < 0;
  }
  template<>
  struct hash<string_view> {
	[[nodiscard]] size_t operator()(const string_view _) const noexcept {
#ifdef _WIN32
	  return std::_Hash_array_representation(_.data(), _.size());
#else
	  return std::_Hash_impl::hash(_.data(), _.size());
#endif // _WIN32
	}
  };
} // namespace std
#endif
#endif