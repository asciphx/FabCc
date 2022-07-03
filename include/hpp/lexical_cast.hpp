#ifndef LEXICAL_CAST_H
#define LEXICAL_CAST_H
#include <type_traits>
#include <string>
#include <string_view>
#include <stdexcept>
#include <string.h>
#pragma warning(disable:4244)
namespace std {
#if defined(_MSC_VER)
#define _INLINE __forceinline
#else
#define _INLINE inline
#endif
  static const std::string STD_TURE("true", 4), STD_FALSE("false", 5);
  template <typename S, typename T> _INLINE S lexical_cast(T& i);
  template <> _INLINE std::string lexical_cast<std::string>(signed char& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned char& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(short& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned short& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(int& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned int& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(long long& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned long long& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(bool& b) { return b ? STD_TURE : STD_FALSE; }
  template <typename S, typename T> inline S lexical_cast(T&& i);
  template <> _INLINE std::string lexical_cast<std::string>(signed char&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned char&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(short&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned short&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(int&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned int&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(long long&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned long long&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(bool&& b) { return b ? STD_TURE : STD_FALSE; }
  static unsigned long STD_POW[10][10] = {
	{0,1,2,3,4,5,6,7,8,9},
	{0,10,20,30,40,50,60,70,80,90},
	{0,100,200,300,400,500,600,700,800,900},
	{0,1000,2000,3000,4000,5000,6000,7000,8000,9000},
	{0,10000,20000,30000,40000,50000,60000,70000,80000,90000},
	{0,100000,200000,300000,400000,500000,600000,700000,800000,900000},
	{0,1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000},
	{0,10000000,20000000,30000000,40000000,50000000,60000000,70000000,80000000,90000000},
	{0,100000000,200000000,300000000,400000000,500000000,600000000,700000000,800000000,900000000},
	{0,1000000000,2000000000,3000000000u,4000000000u,0,0,0,0,0}
  };
  static unsigned long long STD_POWS[20][10] = {
	{0,1,2,3,4,5,6,7,8,9},
	{0,10,20,30,40,50,60,70,80,90},
	{0,100,200,300,400,500,600,700,800,900},
	{0,1000,2000,3000,4000,5000,6000,7000,8000,9000},
	{0,10000,20000,30000,40000,50000,60000,70000,80000,90000},
	{0,100000,200000,300000,400000,500000,600000,700000,800000,900000},
	{0,1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000},
	{0,10000000,20000000,30000000,40000000,50000000,60000000,70000000,80000000,90000000},
	{0,100000000,200000000,300000000,400000000,500000000,600000000,700000000,800000000,900000000},
	{0,1000000000,2000000000,3000000000,4000000000,5000000000,6000000000,7000000000,8000000000,9000000000},
	{0,10000000000,20000000000,30000000000,40000000000,50000000000,60000000000,70000000000,80000000000,90000000000},
	{0,100000000000,200000000000,300000000000,400000000000,500000000000,600000000000,700000000000,800000000000,900000000000},
	{0,1000000000000,2000000000000,3000000000000,4000000000000,5000000000000,6000000000000,7000000000000,8000000000000,9000000000000},
	{0,10000000000000,20000000000000,30000000000000,40000000000000,50000000000000,60000000000000,70000000000000,80000000000000,90000000000000},
	{0,100000000000000,200000000000000,300000000000000,400000000000000,500000000000000,600000000000000,700000000000000,800000000000000,900000000000000},
	{0,1000000000000000,2000000000000000,3000000000000000,4000000000000000,5000000000000000,6000000000000000,7000000000000000,8000000000000000,9000000000000000},
	{0,10000000000000000,20000000000000000,30000000000000000,40000000000000000,50000000000000000,60000000000000000,70000000000000000,80000000000000000,90000000000000000},
	{0,100000000000000000,200000000000000000,300000000000000000,400000000000000000,500000000000000000,600000000000000000,700000000000000000,800000000000000000,900000000000000000},
	{0,1000000000000000000,2000000000000000000,3000000000000000000,4000000000000000000,5000000000000000000,6000000000000000000,7000000000000000000,8000000000000000000,9000000000000000000},
	{0,10000000000000000000ull,0,0,0,0,0,0,0,0},
  };
  //const char*
  template <typename T> _INLINE T lexical_cast(const char* c);
  template <> _INLINE std::string lexical_cast<std::string>(const char* c) { return std::string(c); }
  template <> [[nodiscard]] _INLINE
	bool lexical_cast<bool>(const char* c) {
	if ((c[0] == '1' && c[1] == 0) || (c[0] == 't' && c[1] == 'r' && c[2] == 'u' && c[3] == 'e' && c[4] == 0)) return true;
	if ((c[0] == '0' && c[1] == 0) || (c[0] == 'f' && c[1] == 'a' && c[2] == 'l' && c[3] == 's' && c[4] == 'e' && c[5] == 0))
	  return false; throw std::invalid_argument("");
  }
  template <> [[nodiscard]] _INLINE
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
  template <> [[nodiscard]] _INLINE
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
  template <> [[nodiscard]] _INLINE
	short lexical_cast<short>(const char* c) {
	size_t l = strlen(c); short r = 0;
	if (*c != 0x2D) {
	  if (l < 6) {
		while (l-- > 1) {
		  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r += STD_POW[l][*c++ - 0x30];
		}
		if (*c > 0x37 || 0x30 > *c) throw std::range_error("");
		r += *c - 0x30; return r;
	  } throw std::out_of_range("");
	} else {
	  if (--l < 6) {
		while (l-- > 1) {
		  if (*++c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r -= STD_POW[l][*c - 0x30];
		}
		if (*c > 0x38 || 0x30 > *c) throw std::range_error("");
		r -= *c - 0x30; return r;
	  } throw std::out_of_range("");
	}
  }
  template <> [[nodiscard]] _INLINE
	unsigned short lexical_cast<unsigned short>(const char* c) {
	size_t l = strlen(c); unsigned short r = 0;
	if (l < 6) {
	  while (l-- > 1) {
		if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POW[l][*c++ - 0x30];
	  }
	  if (*c > 0x35 || 0x30 > *c) throw std::range_error("");
	  r += *c - 0x30; return r;
	} throw std::out_of_range("");
  }
  template <> [[nodiscard]] _INLINE
	int lexical_cast<int>(const char* c) {
	size_t l = strlen(c); int r = 0;
	if (*c != 0x2D) {
	  if (l < 11) {
		while (l-- > 1) {
		  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r += STD_POW[l][*c++ - 0x30];
		}
		if (*c > 0x37 || 0x30 > *c) throw std::range_error("");
		r += *c - 0x30; return r;
	  } throw std::out_of_range("");
	} else {
	  if (--l < 11) {
		while (l-- > 1) {
		  if (*++c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r -= STD_POW[l][*c - 0x30];
		}
		if (*c > 0x38 || 0x30 > *c) throw std::range_error("");
		r -= *c - 0x30; return r;
	  } throw std::out_of_range("");
	}
  }
  template <> [[nodiscard]] _INLINE
	unsigned int lexical_cast<unsigned int>(const char* c) {
	size_t l = strlen(c); unsigned int r = 0;
	if (l < 11) {
	  while (l-- > 1) {
		if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POW[l][*c++ - 0x30];
	  }
	  if (*c > 0x35 || 0x30 > *c) throw std::range_error("");
	  r += *c - 0x30; return r;
	} throw std::out_of_range("");
  }
  template <> [[nodiscard]] _INLINE
	long long lexical_cast<long long>(const char* c) {
	size_t l = strlen(c); long long r = 0;
	if (*c != 0x2D) {
	  if (l < 20) {
		while (l-- > 1) {
		  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POWS[l][*c++ - 0x30];
		}
		if (*c > 0x37 || 0x30 > *c) throw std::range_error("");
		r += *c - 0x30; return r;
	  } throw std::out_of_range("");
	} else {
	  if (--l < 20) {
		while (l-- > 1) {
		  if (*++c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r -= STD_POWS[l][*c - 0x30];
		}
		if (*c > 0x38 || 0x30 > *c) throw std::range_error("");
		r -= *c - 0x30; return r;
	  } throw std::out_of_range("");
	}
  }
  template <> [[nodiscard]] _INLINE
	unsigned long long lexical_cast<unsigned long long>(const char* c) {
	size_t l = strlen(c); unsigned long long r = 0;
	if (l < 21) {
	  while (l-- > 1) {
		if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POWS[l][*c++ - 0x30];
	  }
	  if (*c > 0x35 || 0x30 > *c) throw std::range_error("");
	  r += *c - 0x30; return r;
	} throw std::out_of_range("");
  }
  template <> [[nodiscard]] _INLINE
	float lexical_cast<float>(const char* c) {
	char* $; const float _ = ::strtof(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <> [[nodiscard]] _INLINE
	double lexical_cast<double>(const char* c) {
	char* $; const double _ = ::strtod(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <> [[nodiscard]] _INLINE
	long double lexical_cast<long double>(const char* c) {
	char* $; const long double _ = ::strtold(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <> _INLINE
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
  template <typename T> _INLINE T lexical_cast(std::string& s);
  template <> [[nodiscard]] _INLINE
	bool lexical_cast<bool>(std::string& s) {
	if (s == "1" || s == "true")return true; if (s == "0" || s == "false")return false; throw std::invalid_argument("");
  }
  template <> [[nodiscard]] _INLINE
	signed char lexical_cast<signed char>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); signed char r = 0;
	if (*c != 0x2D) {
	  if (l < 4) {
		while (l--) {
		  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r += STD_POW[l][*c++ - 0x30];
		}
		if (*c > 0x37 || 0x30 > *c) throw std::range_error("");
		r += *c - 0x30; return r;
	  } throw std::out_of_range("");
	} else {
	  if (--l < 4) {
		while (l--) {
		  if (*++c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r -= STD_POW[l][*c - 0x30];
		}
		if (*c > 0x38 || 0x30 > *c) throw std::range_error("");
		r -= *c - 0x30; return r;
	  } throw std::out_of_range("");
	}
  }
  template <> [[nodiscard]] _INLINE
	unsigned char lexical_cast<unsigned char>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); unsigned char r = 0;
	if (l < 4) {
	  while (l-- > 1) {
		if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POW[l][*c++ - 0x30];
	  }
	  if (*c > 0x35 || 0x30 > *c) throw std::range_error("");
	  r += *c - 0x30; return r;
	} throw std::out_of_range("");
  }
  template <> [[nodiscard]] _INLINE
	short lexical_cast<short>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); short r = 0;
	if (*c != 0x2D) {
	  if (l < 6) {
		while (l-- > 1) {
		  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r += STD_POW[l][*c++ - 0x30];
		}
		if (*c > 0x37 || 0x30 > *c) throw std::range_error("");
		r += *c - 0x30; return r;
	  } throw std::out_of_range("");
	} else {
	  if (--l < 6) {
		while (l-- > 1) {
		  if (*++c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r -= STD_POW[l][*c - 0x30];
		}
		if (*c > 0x38 || 0x30 > *c) throw std::range_error("");
		r -= *c - 0x30; return r;
	  } throw std::out_of_range("");
	}
  }
  template <> [[nodiscard]] _INLINE
	unsigned short lexical_cast<unsigned short>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); unsigned short r = 0;
	if (l < 6) {
	  while (l-- > 1) {
		if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POW[l][*c++ - 0x30];
	  }
	  if (*c > 0x35 || 0x30 > *c) throw std::range_error("");
	  r += *c - 0x30; return r;
	} throw std::out_of_range("");
  }
  template <> [[nodiscard]] _INLINE
	int lexical_cast<int>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); int r = 0;
	if (*c != 0x2D) {
	  if (l < 11) {
		while (l-- > 1) {
		  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r += STD_POW[l][*c++ - 0x30];
		}
		if (*c > 0x37 || 0x30 > *c) throw std::range_error("");
		r += *c - 0x30; return r;
	  } throw std::out_of_range("");
	} else {
	  if (--l < 11) {
		while (l-- > 1) {
		  if (*++c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r -= STD_POW[l][*c - 0x30];
		}
		if (*c > 0x38 || 0x30 > *c) throw std::range_error("");
		r -= *c - 0x30; return r;
	  } throw std::out_of_range("");
	}
  }
  template <> [[nodiscard]] _INLINE
	unsigned int lexical_cast<unsigned int>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); unsigned int r = 0;
	if (l < 11) {
	  while (l-- > 1) {
		if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POW[l][*c++ - 0x30];
	  }
	  if (*c > 0x35 || 0x30 > *c) throw std::range_error("");
	  r += *c - 0x30; return r;
	} throw std::out_of_range("");
  }
  template <> [[nodiscard]] _INLINE
	long long lexical_cast<long long>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); long long r = 0;
	if (*c != 0x2D) {
	  if (l < 20) {
		while (l-- > 1) {
		  if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r += STD_POWS[l][*c++ - 0x30];
		}
		if (*c > 0x37 || 0x30 > *c) throw std::range_error("");
		r += *c - 0x30; return r;
	  } throw std::out_of_range("");
	} else {
	  if (--l < 20) {
		while (l-- > 1) {
		  if (*++c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		  r -= STD_POWS[l][*c - 0x30];
		}
		if (*c > 0x38 || 0x30 > *c) throw std::range_error("");
		r -= *c - 0x30; return r;
	  } throw std::out_of_range("");
	}
  }
  template <> [[nodiscard]] _INLINE
	unsigned long long lexical_cast<unsigned long long>(std::string& s) {
	const char* c = s.c_str(); size_t l = s.size(); unsigned long long r = 0;
	if (l < 21) {
	  while (l-- > 1) {
		if (*c > 0x39 || 0x30 > *c) throw std::invalid_argument("");
		r += STD_POWS[l][*c++ - 0x30];
	  }
	  if (*c > 0x35 || 0x30 > *c) throw std::range_error("");
	  r += *c - 0x30; return r;
	} throw std::out_of_range("");
  }
  template <> [[nodiscard]] _INLINE
	float lexical_cast<float>(std::string& s) {
	const char* c = s.c_str();
	char* $; const float _ = ::strtof(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <> [[nodiscard]] _INLINE
	double lexical_cast<double>(std::string& s) {
	const char* c = s.c_str();
	char* $; const double _ = ::strtod(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <> [[nodiscard]] _INLINE
	long double lexical_cast<long double>(std::string& s) {
	const char* c = s.c_str();
	char* $; const long double _ = ::strtold(c, &$); if (*$ == 0) return _; throw std::invalid_argument("");
  }
  template <> _INLINE
	tm lexical_cast<tm>(std::string& s) {
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
#undef _INLINE
}
#endif // LEXICAL_CAST_H
