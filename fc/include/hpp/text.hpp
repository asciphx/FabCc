#ifndef TEXT_HPP
#define TEXT_HPP
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
#include <string.h>
#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <iosfwd>
#include <tp/c++.h>
#include <hh/lexical_cast.hh>
const static unsigned char utf$[256] =
{ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7 };
template<unsigned short I = 0x64>//Max [65534(char),16383(utf8)], Min 1, default 100.
class text {//It is similar to a dynamic std::string_view with a fixed maximum length
  unsigned short k = 0, l = 0, u = 0; constexpr const static unsigned short ___ = I * 4; char* _ = new char[___];
  template<unsigned short A, unsigned short K> friend text<A> operator+(text<A>& t, const text<K>& $);
  template<unsigned short B> friend text<B> operator+(const char* c, text<B>& t);
  template<unsigned short C> friend text<C> operator+(const char c, text<C>& t);
  template<unsigned short D> friend text<D> operator+(text<D>& t, const char* c);
  template<unsigned short E> friend text<E> operator+(text<E>& t, const std::string& $);
  friend std::string& operator<<(std::string& s, text<I>* c) {
    s.push_back('"'); s.append(c->_, c->l); s.push_back('"'); return s;
  };
  friend std::string& operator<<(std::string& s, const text<I>* c) {
    s.push_back('"'); s.append(c->_, c->l); s.push_back('"'); return s;
  };
  friend std::string& operator<<(std::string& s, text<I>& c) {
    s.push_back('"'); s.append(c._, c.l); s.push_back('"'); return s;
  };
  friend std::string& operator<<(std::string& s, const text<I>& c) {
    s.push_back('"'); s.append(c._, c.l); s.push_back('"'); return s;
  };
  friend std::ostream& operator<<(std::ostream& s, text<I>& c) { return s << c.c_str(); };
public:
  ~text() { delete[]_; _ = nullptr; };
  text() { assert(I != 0 && I < 0x4000); _[0] = 0; };
  text(const char* s) {
    assert(I != 0 && I < 0x4000); while (*s && k < I) {
      u = utf$[static_cast<unsigned char>(*s)]; while (--u) _[l] = *s++, ++l; ++k;
    } _[l] = 0;
  };
  text(const text& str) { strcpy(_, str._); l = str.l; k = str.k; }
  template<unsigned short L> text(const text<L>& s) { assert(I >= L); strcpy(_, s.c_str()); l = s.length(); k = s.u8size(); }
  text(const std::string& s) {
    while (s[l] && k < I) { u = utf$[static_cast<unsigned char>(s[l])]; while (--u) { _[l] = s[l]; ++l; } ++k; } _[l] = 0;
  }
  text& operator = (const char* s) {
    delete[]_; _ = new char[___]; l = k = 0;
    while (s[l] && k < I) { u = utf$[static_cast<unsigned char>(s[l])]; while (--u) { _[l] = s[l]; ++l; } ++k; } _[l] = 0;
    return *this;
  }
  text& operator = (const std::string& s) {
    delete[]_; _ = new char[___]; l = k = 0;
    while (s[l] && k < I) { u = utf$[static_cast<unsigned char>(s[l])]; while (--u) { _[l] = s[l]; ++l; } ++k; } _[l] = 0;
    return *this;
  }
  text& operator = (const text& s) { delete[]_; _ = new char[___]; strcpy(_, s._); l = s.l; k = s.k; return *this; }
  template<unsigned short L>
  text& operator = (const text<L>& s) {
    delete[]_; _ = new char[___]; strncpy(_, s.c_str(), I * 4 + 2); l = s.length(); k = s.u8size(); _[l] = 0; return *this;
  }
  _FORCE_INLINE const char* c_str() const { return _; }
  _FORCE_INLINE const char* data() const { return _; }
  _FORCE_INLINE const unsigned short length() const { return l; }
  _FORCE_INLINE const unsigned short size() const { return l; }
  _FORCE_INLINE const unsigned short u8size() const { return k; }//utf8Size
  _FORCE_INLINE char& operator()(unsigned short i) { return i < l ? _[i] : _[l]; }//Same as std::string
  text<1> operator[](unsigned short i) {
    text<1> txt; if (i > k)return txt; unsigned short h = 0, g = 0;
    const unsigned char* f = reinterpret_cast<unsigned char*>(_);
    while (h < i) u = utf$[*f] - 1, ++h, f += u; u = utf$[*f];
    while (++g < u) { txt += *f++; } txt += (char)0; return txt;
  }
  void operator +=(const char* c) {
    while (*c && k < I) { u = utf$[static_cast<unsigned char>(*c)]; while (--u) { _[l++] = *c++; } ++k; } _[l] = 0;
  }//Safe, like std::string
  _FORCE_INLINE void operator +=(char c) { _[l++] = c; ++k; }//Incomplete safety, but it's generally safe and fast(maybe need .end())
  _FORCE_INLINE void operator <<(char c) { _[l++] = c; _[l] = 0; ++k; }//Safety
  void operator <<(const char* c) { while (*c && k < ___) { _[l++] = *c++; k = l; } _[l] = 0; }//NOT SAFE,BUT ONLY NOT UTF8
  void operator <<(const std::string s) {
    const char* c = s.c_str(); while (*c && k < ___) { _[l++] = *c++; k = l; } _[l] = 0;
  }//NOT SAFE,BUT ONLY NOT UTF8
  void operator += (const text& t) {
    const char* s = t.c_str();
    if (&t == this) {
      int i = 2 * k, p = 0; if (i > I)i = I; while (k < i) {
        u = utf$[static_cast<unsigned char>(s[p])]; while (--u) { _[l++] = s[p++]; } ++k;
      } _[l] = 0;
    } else {
      unsigned short i = 0; while (s[i] && k < I) {
        u = utf$[static_cast<unsigned char>(s[i])]; while (--u) { _[l++] = s[i++]; } ++k;
      } _[l] = 0;
    }
  }
  template<unsigned short L>
  void operator += (const text<L>& t) {
    const char* s = t.c_str(); while (*s && k < I) {
      u = utf$[static_cast<unsigned char>(*s)]; while (--u) { _[l++] = *s++; } ++k;
    } _[l] = 0;
  }
  void operator += (const std::string& t) {
    const char* s = t.c_str(); while (*s && k < I) {
      u = utf$[static_cast<unsigned char>(*s)]; while (--u) { _[l++] = *s++; } ++k;
    } _[l] = 0;
  }
  _FORCE_INLINE void push_back(const char c) { if (k < I) _[l++] = c, ++k; }
  _FORCE_INLINE void end() { _[l] = 0; }
  _FORCE_INLINE void clear() { _[0] = l = k = 0; }
  void push_begin(const char c) { unsigned short i = l; while (i) { _[i] = _[i - 1]; --i; } _[++l] = 0; _[0] = c, ++k; }
  unsigned short find(const char* b) {
    u = 0; unsigned short L = 0, i = 0; while (b[++L]);
    while (_[++u]) {
      if (_[u] != b[i++])i = 0;
      if (i == L) {
        u -= L;//i = 0;
        return u;
      }
    }
    return 0;
  }
};
template<unsigned short I>
std::ostream& operator<<(std::ostream& s, text<I> c) { return s << c.c_str(); }
template<unsigned short I>
text<I> operator+(text<I>& t, const char* c) {
  unsigned short& l = *(static_cast<unsigned short*>(&t)), & k = *(static_cast<unsigned short*>(&t
    + (char)(&reinterpret_cast<char const volatile&>(((text<I>*)0)->* & text<I>::l)))); while (*c && l < I) {
    unsigned char u = text<I>::utf$[static_cast<unsigned char>(*c)];
    while (--u) t(k) = *c++, ++k; ++l;
  } t(k) = 0; return t;
}
template<unsigned short I>
text<I> operator+(text<I>& t, const std::string& $) {
  unsigned short& l = *(static_cast<unsigned short*>(&t)), & k = *(static_cast<unsigned short*>(&t
    + (char)(&reinterpret_cast<char const volatile&>(((text<I>*)0)->* & text<I>::l))));
  char* c = (char*)$.c_str(); while (*c && l < I) {
    unsigned char u = text<I>::utf$[static_cast<unsigned char>(*c)];
    while (--u) t(k) = *c++, ++k; ++l;
  } t(k) = 0; return t;
}
template<unsigned short I, unsigned short K>
text<I> operator+(text<I>& t, const text<K>& $) {
  unsigned short& l = *(static_cast<unsigned short*>(&t)), & k = *(static_cast<unsigned short*>(&t
    + (char)(&reinterpret_cast<char const volatile&>(((text<I>*)0)->* & text<I>::l))));
  char* c = (char*)$.c_str(); while (*c && l < I) {
    unsigned char u = text<I>::utf$[static_cast<unsigned char>(*c)];
    while (--u) t(k) = *c++, ++k; ++l;
  } t(k) = 0; return t;
}
template<unsigned short I>
std::string operator+(std::string& t, const text<I>& $) {
  char* c = (char*)$.c_str(); while (*c) { t.push_back(*c); ++c; } t.push_back(0); return t;
}
template<unsigned short I>
text<I> operator+(const char* c, text<I>& t) {
  text<I> f; unsigned short& l = *(static_cast<unsigned short*>(&f)), & k = *(static_cast<unsigned short*>(&f
    + (char)(&reinterpret_cast<char const volatile&>(((text<I>*)0)->* & text<I>::l))));
  while (*c && l < I) { unsigned char u = text<I>::utf$[static_cast<unsigned char>(*c)]; while (--u)f(k) = *c++, ++k; ++l; }
  f += t; return f;
}
template<unsigned short I>
text<I> operator+(const char c, text<I>& t) {
  unsigned short& l = *(static_cast<unsigned short*>(&t)), & k = *(static_cast<unsigned short*>(&t
    + (char)(&reinterpret_cast<char const volatile&>(((text<I>*)0)->* & text<I>::l)))), i = k;
  while (i) { t(i) = t(i - 1); --i; } ++l; t(0) = c; t(++k) = 0; return t;
}
template<class T>
struct is_text: std::false_type {};
template<class T>
struct is_text<T[]>: std::false_type {};
template<unsigned short N>
struct is_text<text<N>>: std::true_type {};
_FORCE_INLINE const char* textify(const char* t) { return t; }
_FORCE_INLINE const char* textify(const std::string& t) { return t.c_str(); }
_FORCE_INLINE const std::string textify(const tm& _v) {
  std::ostringstream os; os << std::setfill('0');
#ifdef _WIN32
  os << std::setw(4) << _v.tm_year + 1900;
#else
  int y = _v.tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
  os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
    << _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec; return os.str();
}
template<unsigned short I>
_FORCE_INLINE const char* textify(const text<I>& t) { return t.c_str(); }
template<typename T>
_FORCE_INLINE typename std::enable_if<std::is_fundamental<T>::value, const std::string>::type textify(const T& t) {
  return std::lexical_cast<std::string>(t);
}
template<unsigned short I, typename T>
text<I * 2> operator^(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '<'; x += '>';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator%(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '=';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator<(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '<';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator<=(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '<'; x += '=';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator>=(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '>'; x += '=';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, typename T>
text<I * 2> operator>(const text<I>& o, const T& v) {
  text<I * 2> x(o); x += '>';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<unsigned short I, unsigned short L>
text<I + L + 7> operator&&(text<I> o, const text<L>& c) {
  text<I + L + 7> x; x += '('; x += o; x += " AND "; x += c; x += ')'; x.end(); return x;
};
template<unsigned short I, unsigned short L>
text<I + L + 6> operator||(text<I> o, const text<L>& c) {
  text<I + L + 6> x; x += '('; x += o; x += " OR "; x += c; x += ')'; x.end(); return x;
};
template<unsigned short I, typename T>
std::string operator>>(text<I> o, const std::vector<T>& c) {
  std::string s(o.c_str()); s += " IN("; size_t l = c.size(); if (l > 0) {
    s += textify(c[0]); for (size_t i = 1; i < l; ++i) { s += ',', s += textify(c[i]); }
  } s.push_back(')'); return s;
};
//for sql key
template<typename T>
text<92> operator>=(const std::string_view& o, const T& v) {
  text<92> x(o.data()); x += '>'; x += '=';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<typename T>
text<92> operator>(const std::string_view& o, const T& v) {
  text<92> x(o.data()); x += '>';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<typename T>
text<92> operator<(const std::string_view& o, const T& v) {
  text<92> x(o.data()); x += '<';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<typename T>
text<92> operator<=(const std::string_view& o, const T& v) {
  text<92> x(o.data()); x += '<'; x += '=';
  if __CONSTEXPR(std::is_same<T, std::string>::value || std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<typename T>
text<92> operator^(const std::string_view& o, const T& v) {
  text<92> x(o.data()); x += '<'; x += '>';
  if __CONSTEXPR(std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
template<typename T>
text<92> operator%(const std::string_view& o, const T& v) {
  text<92> x(o.data()); x += '=';
  if __CONSTEXPR(std::is_same<T, tm>::value || std::is_same<T, const char*>::value) {
    x += '\''; x += textify(v); x += '\'';
  } else {
    x += textify(v);
  } x += (char)0; return x;
}
#endif // !TEXT_HPP
