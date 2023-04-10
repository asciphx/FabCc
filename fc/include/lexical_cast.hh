#ifndef LEXICAL_CAST_H
#define LEXICAL_CAST_H
/*
* This software is licensed under the AGPL-3.0 License.
*
* Copyright (C) 2023 Asciphx
*
* Permissions of this strongest copyleft license are conditioned on making available
* complete source code of licensed works and modifications, which include larger works
* using a licensed work, under the same license. Copyright and license notices must be
* preserved. Contributors provide an express grant of patent rights. When a modified
* version is used to provide a service over a network, the complete source code of
* the modified version must be made available.
*/
#include <type_traits>
#include <stdexcept>
#include <string>
#include <hpp/string_view.hpp>
#include <tp/jeaiii.hpp>
#include <h/dtoa_milo.h>
#include <buf.hh>
namespace std {
#if !defined(_NODISCARD)
#define _NODISCARD [[nodiscard]]
#endif
#if defined(_MSC_VER)
#define _INLINE __forceinline
#else
#define _INLINE inline __attribute__((always_inline))
#endif
  static const string STD_TURE("true", 4), STD_FALSE("false", 5);

  template <typename T> _INLINE T lexical_cast(const char* c);
  //template <typename S> _INLINE S lexical_cast(string_view sv) { return lexical_cast<S>(sv.data()); };
  template <> string lexical_cast<string>(const char* c);
  template <> _NODISCARD bool lexical_cast<bool>(const char* c);
  template <> _NODISCARD char lexical_cast<char>(const char* c);
  template <> _NODISCARD signed char lexical_cast<signed char>(const char* c);
  template <> _NODISCARD unsigned char lexical_cast<unsigned char>(const char* c);
  template <> _NODISCARD short lexical_cast<short>(const char* c);
  template <> _NODISCARD unsigned short lexical_cast<unsigned short>(const char* c);
  template <> _NODISCARD int lexical_cast<int>(const char* c);
  template <> _NODISCARD unsigned int lexical_cast<unsigned int>(const char* c);
  template <> _NODISCARD long lexical_cast<long>(const char* c);
  template <> _NODISCARD long unsigned lexical_cast<long unsigned>(const char* c);
  template <> _NODISCARD long long lexical_cast<long long>(const char* c);
  template <> _NODISCARD unsigned long long lexical_cast<unsigned long long>(const char* c);
  template <> _NODISCARD float lexical_cast<float>(const char* c);
  template <> _NODISCARD double lexical_cast<double>(const char* c);
  template <> _NODISCARD long double lexical_cast<long double>(const char* c);
  template <> tm lexical_cast<tm>(const char* c);

  template <typename T> _INLINE T lexical_cast(char* c);
  template <> string lexical_cast<string>(char* c);
  template <> _NODISCARD bool lexical_cast<bool>(char* c);
  template <> _NODISCARD char lexical_cast<char>(char* c);
  template <> _NODISCARD signed char lexical_cast<signed char>(char* c);
  template <> _NODISCARD unsigned char lexical_cast<unsigned char>(char* c);
  template <> _NODISCARD short lexical_cast<short>(char* c);
  template <> _NODISCARD unsigned short lexical_cast<unsigned short>(char* c);
  template <> _NODISCARD int lexical_cast<int>(char* c);
  template <> _NODISCARD unsigned int lexical_cast<unsigned int>(char* c);
  template <> _NODISCARD long lexical_cast<long>(char* c);
  template <> _NODISCARD long unsigned lexical_cast<long unsigned>(char* c);
  template <> _NODISCARD long long lexical_cast<long long>(char* c);
  template <> _NODISCARD unsigned long long lexical_cast<unsigned long long>(char* c);
  template <> _NODISCARD float lexical_cast<float>(char* c);
  template <> _NODISCARD double lexical_cast<double>(char* c);
  template <> _NODISCARD long double lexical_cast<long double>(char* c);
  template <> tm lexical_cast<tm>(char* c);

  template <typename T> _INLINE T lexical_cast(string& s);
  template <> _NODISCARD bool lexical_cast<bool>(string& s);
  template <> _NODISCARD char lexical_cast<char>(string& s);
  template <> _NODISCARD signed char lexical_cast<signed char>(string& s);
  template <> _NODISCARD unsigned char lexical_cast<unsigned char>(string& s);
  template <> _NODISCARD short lexical_cast<short>(string& s);
  template <> _NODISCARD unsigned short lexical_cast<unsigned short>(string& s);
  template <> _NODISCARD int lexical_cast<int>(string& s);
  template <> _NODISCARD unsigned int lexical_cast<unsigned int>(string& s);
  template <> _NODISCARD long lexical_cast<long>(string& s);
  template <> _NODISCARD long unsigned lexical_cast<long unsigned>(string& s);
  template <> _NODISCARD long long lexical_cast<long long>(string& s);
  template <> _NODISCARD unsigned long long lexical_cast<unsigned long long>(string& s);
  template <> _NODISCARD float lexical_cast<float>(string& s);
  template <> _NODISCARD double lexical_cast<double>(string& s);
  template <> _NODISCARD long double lexical_cast<long double>(string& s);
  template <> tm lexical_cast<tm>(string& s);

  template <typename T> _INLINE T lexical_cast(string&& s);
  template <> _NODISCARD bool lexical_cast<bool>(string&& s);
  template <> _NODISCARD char lexical_cast<char>(string&& s);
  template <> _NODISCARD signed char lexical_cast<signed char>(string&& s);
  template <> _NODISCARD unsigned char lexical_cast<unsigned char>(string&& s);
  template <> _NODISCARD short lexical_cast<short>(string&& s);
  template <> _NODISCARD unsigned short lexical_cast<unsigned short>(string&& s);
  template <> _NODISCARD int lexical_cast<int>(string&& s);
  template <> _NODISCARD unsigned int lexical_cast<unsigned int>(string&& s);
  template <> _NODISCARD long lexical_cast<long>(string&& s);
  template <> _NODISCARD long unsigned lexical_cast<long unsigned>(string&& s);
  template <> _NODISCARD long long lexical_cast<long long>(string&& s);
  template <> _NODISCARD unsigned long long lexical_cast<unsigned long long>(string&& s);
  template <> _NODISCARD float lexical_cast<float>(string&& s);
  template <> _NODISCARD double lexical_cast<double>(string&& s);
  template <> _NODISCARD long double lexical_cast<long double>(string&& s);
  template <> tm lexical_cast<tm>(string&& s);

  template <typename T> _INLINE T lexical_cast(string_view& s);
  template <> _NODISCARD string lexical_cast<string>(string_view& c);
  template <> _NODISCARD bool lexical_cast<bool>(string_view& s);
  template <> _NODISCARD char lexical_cast<char>(string_view& s);
  template <> _NODISCARD signed char lexical_cast<signed char>(string_view& s);
  template <> _NODISCARD unsigned char lexical_cast<unsigned char>(string_view& s);
  template <> _NODISCARD short lexical_cast<short>(string_view& s);
  template <> _NODISCARD unsigned short lexical_cast<unsigned short>(string_view& s);
  template <> _NODISCARD int lexical_cast<int>(string_view& s);
  template <> _NODISCARD unsigned int lexical_cast<unsigned int>(string_view& s);
  template <> _NODISCARD long lexical_cast<long>(string_view& s);
  template <> _NODISCARD long unsigned lexical_cast<long unsigned>(string_view& s);
  template <> _NODISCARD long long lexical_cast<long long>(string_view& s);
  template <> _NODISCARD unsigned long long lexical_cast<unsigned long long>(string_view& s);
  template <> _NODISCARD float lexical_cast<float>(string_view& s);
  template <> _NODISCARD double lexical_cast<double>(string_view& s);
  template <> _NODISCARD long double lexical_cast<long double>(string_view& s);
  template <> tm lexical_cast<tm>(string_view& s);

  template <typename T> _INLINE T lexical_cast(string_view&& s);
  template <> _NODISCARD string lexical_cast<string>(string_view&& c);
  template <> _NODISCARD bool lexical_cast<bool>(string_view&& s);
  template <> _NODISCARD char lexical_cast<char>(string_view&& s);
  template <> _NODISCARD signed char lexical_cast<signed char>(string_view&& s);
  template <> _NODISCARD unsigned char lexical_cast<unsigned char>(string_view&& s);
  template <> _NODISCARD short lexical_cast<short>(string_view&& s);
  template <> _NODISCARD unsigned short lexical_cast<unsigned short>(string_view&& s);
  template <> _NODISCARD int lexical_cast<int>(string_view&& s);
  template <> _NODISCARD unsigned int lexical_cast<unsigned int>(string_view&& s);
  template <> _NODISCARD long lexical_cast<long>(string_view&& s);
  template <> _NODISCARD long unsigned lexical_cast<long unsigned>(string_view&& s);
  template <> _NODISCARD long long lexical_cast<long long>(string_view&& s);
  template <> _NODISCARD unsigned long long lexical_cast<unsigned long long>(string_view&& s);
  template <> _NODISCARD float lexical_cast<float>(string_view&& s);
  template <> _NODISCARD double lexical_cast<double>(string_view&& s);
  template <> _NODISCARD long double lexical_cast<long double>(string_view&& s);
  template <> tm lexical_cast<tm>(string_view&& s);

  template <typename S, typename T> _INLINE S lexical_cast(T& i);
  template <> inline string lexical_cast<string>(char& i) { char s[4]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(signed char& i) { char s[4]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(unsigned char& i) { char s[3]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(short& i) { char s[6]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(unsigned short& i) { char s[5]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(int& i) { char s[11]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(unsigned int& i) { char s[10]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(long& i) { char s[11]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(long unsigned& i) { char s[10]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(long long& i) { char s[20]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(unsigned long long& i) { char s[20]; return string(s, jeaiii::to_text_from_integer(s, i) - s); }
  template <> inline string lexical_cast<string>(bool& b) { return b ? STD_TURE : STD_FALSE; }
  template <> inline string lexical_cast<string>(double& f) { char s[16]; return string(s, milo::dtoa(f, s, 0x10)); }
  template <> inline string lexical_cast<string>(float& f) { char s[7]; return string(s, milo::dtoa(f, s, 0x7)); }
  template <> inline string lexical_cast<string>(long double& f) { return to_string(f); }//char s[34]; return string(s, milo::dtoa(f, s, 0x22));
  template <> inline string lexical_cast<string>(string& c) { return c; }
  template <typename S, typename T> _INLINE S lexical_cast(T&& i);
  template <> inline string lexical_cast<string>(char&& i) { char s[4]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(signed char&& i) { char s[4]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(unsigned char&& i) { char s[3]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(short&& i) { char s[6]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(unsigned short&& i) { char s[5]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(int&& i) { char s[11]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(unsigned int&& i) { char s[10]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(long&& i) { char s[11]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(long unsigned&& i) { char s[10]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(long long&& i) { char s[20]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(unsigned long long&& i) { char s[20]; return string(s, jeaiii::to_text_from_integer(s, std::move(i)) - s); }
  template <> inline string lexical_cast<string>(bool&& b) { return b ? STD_TURE : STD_FALSE; }
  template <> inline string lexical_cast<string>(double&& f) { char s[16]; return string(s, milo::dtoa(std::move(f), s, 0x10)); }
  template <> inline string lexical_cast<string>(float&& f) { char s[7]; return string(s, milo::dtoa(std::move(f), s, 0x7)); }
  template <> inline string lexical_cast<string>(long double&& f) { return to_string(std::move(f)); }//milo not support
  template <> inline string lexical_cast<string>(string&& c) { return c; }
}
#undef _INLINE
#endif // LEXICAL_CAST_H
