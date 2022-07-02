#include <type_traits>
#include <string>
#include <string_view>
#include <stdexcept>
namespace std {
  static const std::string STD_TURE("true", 4), STD_FALSE("false", 5);
  template <typename S, typename T> S lexical_cast(T& i) { throw std::overflow_error(""); }
  template <> std::string lexical_cast<std::string>(signed char& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned char& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(short& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned short& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(int& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned int& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(long long& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned long long& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(bool& b) { return b ? STD_TURE : STD_FALSE; }
  template <typename S, typename T> S lexical_cast(T&& i) { throw std::overflow_error(""); }
  template <> std::string lexical_cast<std::string>(signed char&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned char&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(short&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned short&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(int&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned int&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(long long&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(unsigned long long&& i) { return std::to_string(i); }
  template <> std::string lexical_cast<std::string>(bool&& b) { return b ? STD_TURE : STD_FALSE; }
  //const char*
  template <typename T> T lexical_cast(const char* c) { throw std::overflow_error(""); }
  template <> std::string lexical_cast<std::string>(const char* c) { return std::string(c); }
  template <>
  bool lexical_cast<bool>(const char* c) {
	if ((c[0] == '1' && c[1] == 0) || (c[0] == 't' && c[1] == 'r' && c[2] == 'u' && c[3] == 'e' && c[4] == 0)) return true;
	if ((c[0] == '0' && c[1] == 0) || (c[0] == 'f' && c[1] == 'a' && c[2] == 'l' && c[3] == 's' && c[4] == 'e' && c[5] == 0))
	  return false; throw std::invalid_argument("");
  }
  template <>
  signed char lexical_cast<signed char>(const char* c) {
	signed char r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned char lexical_cast<unsigned char>(const char* c) {
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned char r = *c - 0x30; char z = 0;
	while (z != 1 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 1 && *++c) {
	  if (r > 25 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  short lexical_cast<short>(const char* c) {
	short r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned short lexical_cast<unsigned short>(const char* c) {
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned short r = *c - 0x30; char z = 0;
	while (z != 3 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 3 && *++c) {
	  if (r > 6553 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  int lexical_cast<int>(const char* c) {
	int r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned int lexical_cast<unsigned int>(const char* c) {
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned int r = *c - 0x30; char z = 0;
	while (z != 8 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 8 && *++c) {
	  if (r > 429496729 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  long long lexical_cast<long long>(const char* c) {
	long long r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned long long lexical_cast<unsigned long long>(const char* c) {
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned long long r = *c - 0x30; char z = 0;
	while (z != 18 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 18 && *++c) {
	  if (r > 1844674407370955161 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  float lexical_cast<float>(const char* c) {
	char* $; const float _ = ::strtof(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <>
  double lexical_cast<double>(const char* c) {
	char* $; const double _ = ::strtod(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <>
  long double lexical_cast<long double>(const char* c) {
	char* $; const long double _ = ::strtold(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <>
  tm lexical_cast<tm>(const char* c) {
	int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0; tm t;
	if (sscanf(c, "%4d-%2d-%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min, &sec) == 6) {
	  t.tm_mday = day;
	  t.tm_hour = hour;
	  t.tm_min = min;
	  t.tm_sec = sec;
	}
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	return t;
  }
  //std::string
  template <typename T> T lexical_cast(const std::string& s) {
	static_assert(std::is_convertible<std::string, T>::value, "Impossible to convert T"); throw std::overflow_error("");
  }
  template <>
  bool lexical_cast<bool>(const std::string& s) {
	const char* c = s.c_str();
	if ((c[0] == '1' && c[1] == 0) || (c[0] == 't' && c[1] == 'r' && c[2] == 'u' && c[3] == 'e' && c[4] == 0)) return true;
	if ((c[0] == '0' && c[1] == 0) || (c[0] == 'f' && c[1] == 'a' && c[2] == 'l' && c[3] == 's' && c[4] == 'e' && c[5] == 0))
	  return false; throw std::invalid_argument("");
  }
  template <>
  signed char lexical_cast<signed char>(const std::string& s) {
	const char* c = s.c_str();
	signed char r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned char lexical_cast<unsigned char>(const std::string& s) {
	const char* c = s.c_str();
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned char r = *c - 0x30; char z = 0;
	while (z != 1 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 1 && *++c) {
	  if (r > 25 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  short lexical_cast<short>(const std::string& s) {
	const char* c = s.c_str();
	short r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned short lexical_cast<unsigned short>(const std::string& s) {
	const char* c = s.c_str();
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned short r = *c - 0x30; char z = 0;
	while (z != 3 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 3 && *++c) {
	  if (r > 6553 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  int lexical_cast<int>(const std::string& s) {
	const char* c = s.c_str();
	int r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned int lexical_cast<unsigned int>(const std::string& s) {
	const char* c = s.c_str();
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned int r = *c - 0x30; char z = 0;
	while (z != 8 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 8 && *++c) {
	  if (r > 429496729 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  long long lexical_cast<long long>(const std::string& s) {
	const char* c = s.c_str();
	long long r; if (*c != 0x2D) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = *c - 0x30;
	  while (*++c) {
		r = r * 10 + *c - 0x30; if (r <= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  }
	} else {
	  r = 0x30 - *++c; if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
	  while (*++c) {
		r = r * 10 - *c + 0x30; if (r >= 0 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  };
	} return r;
  }
  template <>
  unsigned long long lexical_cast<unsigned long long>(const std::string& s) {
	const char* c = s.c_str();
	if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); unsigned long long r = *c - 0x30; char z = 0;
	while (z != 18 && *++c) {
	  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument(""); r = r * 10 + *c - 0x30; ++z;
	}
	if (z == 18 && *++c) {
	  if (r > 1844674407370955161 || *c > 0x39 || 0x30 > *c) throw std::range_error("");
	  r = r * 10 + *c - 0x30;
	  if (10 > r) throw std::out_of_range("");
	} return r;
  }
  template <>
  float lexical_cast<float>(const std::string& s) {
	const char* c = s.c_str();
	char* $; const float _ = ::strtof(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <>
  double lexical_cast<double>(const std::string& s) {
	const char* c = s.c_str();
	char* $; const double _ = ::strtod(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <>
  long double lexical_cast<long double>(const std::string& s) {
	const char* c = s.c_str();
	char* $; const long double _ = ::strtold(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <>
  tm lexical_cast<tm>(const std::string& s) {
	const char* c = s.c_str();
	int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0; tm t;
	if (sscanf(c, "%4d-%2d-%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min, &sec) == 6) {
	  t.tm_mday = day;
	  t.tm_hour = hour;
	  t.tm_min = min;
	  t.tm_sec = sec;
	}
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	return t;
  }
}