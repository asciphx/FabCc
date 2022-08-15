#ifndef LEXICAL_CAST_H
#define LEXICAL_CAST_H
#include <type_traits>
#include <string>
#include <string_view>
#include <stdexcept>
namespace std {
#if defined(_MSC_VER) && !defined(_INLINE)
#define _INLINE __forceinline
#elif !defined(_INLINE)
#define _INLINE __attribute__((always_inline))
#endif
  static const std::string STD_TURE("true", 4), STD_FALSE("false", 5);
  template <typename S, typename T> _INLINE S lexical_cast(T& i);
  template <> _INLINE std::string lexical_cast<std::string>(char& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(signed char& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned char& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(short& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned short& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(int& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned int& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(long long& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned long long& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(bool& b) { return b ? STD_TURE : STD_FALSE; }
  template <> _INLINE std::string lexical_cast<std::string>(double& f) { return std::to_string(f); }
  template <> _INLINE std::string lexical_cast<std::string>(float& f) { return std::to_string(f); }
  template <> _INLINE std::string lexical_cast<std::string>(long double& f) { return std::to_string(f); }
  template <> _INLINE std::string lexical_cast<std::string>(std::string& c) { return c; }
  template <typename S, typename T> _INLINE S lexical_cast(T&& i);
  template <> _INLINE std::string lexical_cast<std::string>(char&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(signed char&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned char&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(short&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned short&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(int&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned int&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(long long&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(unsigned long long&& i) { return std::to_string(i); }
  template <> _INLINE std::string lexical_cast<std::string>(bool&& b) { return b ? STD_TURE : STD_FALSE; }
  template <> _INLINE std::string lexical_cast<std::string>(double&& f) { return std::to_string(f); }
  template <> _INLINE std::string lexical_cast<std::string>(float&& f) { return std::to_string(f); }
  template <> _INLINE std::string lexical_cast<std::string>(long double&& f) { return std::to_string(f); }
  template <> _INLINE std::string lexical_cast<std::string>(std::string&& c) { return c; }

  template <typename T> T lexical_cast(const char* c);
  template <> std::string lexical_cast<std::string>(const char* c);
  template <> [[nodiscard]] bool lexical_cast<bool>(const char* c);
  template <> [[nodiscard]] char lexical_cast<char>(const char* c);
  template <> [[nodiscard]] signed char lexical_cast<signed char>(const char* c);
  template <> [[nodiscard]] unsigned char lexical_cast<unsigned char>(const char* c);
  template <> [[nodiscard]] short lexical_cast<short>(const char* c);
  template <> [[nodiscard]] unsigned short lexical_cast<unsigned short>(const char* c);
  template <> [[nodiscard]] int lexical_cast<int>(const char* c);
  template <> [[nodiscard]] unsigned int lexical_cast<unsigned int>(const char* c);
  template <> [[nodiscard]] long long lexical_cast<long long>(const char* c);
  template <> [[nodiscard]] unsigned long long lexical_cast<unsigned long long>(const char* c);
  template <> [[nodiscard]] float lexical_cast<float>(const char* c);
  template <> [[nodiscard]] double lexical_cast<double>(const char* c);
  template <> [[nodiscard]] long double lexical_cast<long double>(const char* c);
  template <> tm lexical_cast<tm>(const char* c);
  template <typename S> _INLINE S lexical_cast(std::string_view sv) { return std::lexical_cast<S>(sv.data()); };
  template <typename T> T lexical_cast(std::string& s);
  template <> [[nodiscard]] bool lexical_cast<bool>(std::string& s);
  template <> [[nodiscard]] char lexical_cast<char>(std::string& s);
  template <> [[nodiscard]] signed char lexical_cast<signed char>(std::string& s);
  template <> [[nodiscard]] unsigned char lexical_cast<unsigned char>(std::string& s);
  template <> [[nodiscard]] short lexical_cast<short>(std::string& s);
  template <> [[nodiscard]] unsigned short lexical_cast<unsigned short>(std::string& s);
  template <> [[nodiscard]] int lexical_cast<int>(std::string& s);
  template <> [[nodiscard]] unsigned int lexical_cast<unsigned int>(std::string& s);
  template <> [[nodiscard]] long long lexical_cast<long long>(std::string& s);
  template <> [[nodiscard]] unsigned long long lexical_cast<unsigned long long>(std::string& s);
  template <> [[nodiscard]] float lexical_cast<float>(std::string& s);
  template <> [[nodiscard]] double lexical_cast<double>(std::string& s);
  template <> [[nodiscard]] long double lexical_cast<long double>(std::string& s);
  template <> tm lexical_cast<tm>(std::string& s);
}
#endif // LEXICAL_CAST_H
