#ifndef JSON_H
#define JSON_H
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-bool-conversion"
#endif
#ifdef _MSC_VER
#pragma warning (disable:4200)
#else
#include <math.h>
#endif
#include "str.hh"
#include "hpp/string_view.hpp"
#include "h/macros.h"
#include "hpp/box.hpp"
#include "hpp/tuple.hpp"
#include "hpp/text.hpp"
#include "hh/lexical_cast.hh"
#ifndef _MSVC_LANG
#include <sys/stat.h>
#endif
#include <cassert>
#include <initializer_list>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <tuple>
#include <vector>
#include <chrono>
#include <fstream>
namespace json {
  static const std::string RES_NULL("null", 4);
  namespace xx {
    struct _H { u32 cap; u32 size; void* p[]; };
    class Array {
      _H* _h;
    public:
      static constexpr const u32 N = _PTR_LEN, R = sizeof(_H) / _PTR_LEN, Z = 1024 - R; Array(): Array(Z) {} ~Array() { ::free(_h); }
      _FORCE_INLINE explicit Array(u32 cap) { _h = (_H*) ::malloc(N * (R + cap)); _h->cap = cap; _h->size = 0; }
      _FORCE_INLINE bool empty() const { return this->_h->size == 0; } _FORCE_INLINE void resize(u32 n) { _h->size = n; }
      _FORCE_INLINE void** data() const { return _h->p; } _FORCE_INLINE u32 size() const { return _h->size; }
      _FORCE_INLINE void* pop_back() { return _h->p[--_h->size]; } _FORCE_INLINE void*& back() const { return _h->p[this->_h->size - 1]; }
      _FORCE_INLINE void*& operator[](u32 i) const { return _h->p[i]; }
      void push_back(void* v) {
        if (_h->size == _h->cap) { const size_t n = N * _h->cap, o = sizeof(_H) + n; _h = (_H*) ::realloc(_h, o + n); _h->cap <<= 1; }
        _h->p[_h->size++] = v;
      }
      _FORCE_INLINE void remove(u32 i) { if (i != --_h->size) _h->p[i] = _h->p[_h->size]; }
      void remove_pair(u32 i) { if (i != (_h->size -= 2)) { _h->p[i] = _h->p[_h->size]; _h->p[i + 1] = _h->p[_h->size + 1]; } }
      void erase(u32 i) { if (i != --_h->size) { memmove(_h->p + i, _h->p + i + 1, (_h->size - i) * N); } }
      void erase_pair(u32 i) { if (i != (_h->size -= 2)) { memmove(_h->p + i, _h->p + i + 2, (_h->size - i) * N); } }
      _FORCE_INLINE void reset() { _h->size = 0; if (_h->cap > 8192) { ::free(_h); new(this) Array(); } }
    };
    void* alloc(); char* alloc_string(const void* p, size_t n);
  } // xx
  class Json {
  public:
    enum { t_bool = 0, t_int = 1, t_uint = 2, t_double = 4, t_string = 8, t_array = 16, t_object = 32 }; struct _obj_t {}; struct _arr_t {};
    struct _H {
      _H(bool v) noexcept: type(t_bool), b(v) {} _H(i64& v) noexcept: type(t_int), i(v) {} _H(i64&& v) noexcept: type(t_int), i(std::move(v)) {}
      _H(u64& v) noexcept: type(t_uint), i(static_cast<i64>(v)) {} _H(u64&& v) noexcept: type(t_uint), i(std::move(static_cast<i64>(v))) {}
      _H(f64& v) noexcept: type(t_double), d(v) {} _H(f64&& v) noexcept: type(t_double), d(std::move(v)) {} _H(const char* p): _H(p, strlen(p)) {}
      _H(_obj_t&&) noexcept: type(t_object), p(0) {} _H(_arr_t&&) noexcept: type(t_array), p(0) {}
      _H(const void* p, size_t&& n): type(t_string), size(std::move(static_cast<u32&&>(n))) { s = xx::alloc_string(p, size); }
      _H(const void* p, size_t& n): type(t_string), size(static_cast<u32>(n)) { s = xx::alloc_string(p, size); }
      u32 type, size; // size of string
      union {
        bool b;   // for bool
        i64 i;  // for number
        f64 d; // for f64
        char* s;  // for string
        void* p;  // for array and object
      };
    };
    Json() noexcept: _h(0) {} Json(decltype(null)) noexcept: _h(0) {}
    Json(Json&& v) noexcept: _h(v._h) { v._h = 0; } Json(Json& v) noexcept: _h(v._h) { v._h = 0; }
    ~Json() { if (_h) this->reset(); } Json(const Json& v) = delete; void operator=(const Json&) = delete;
    Json& operator=(Json&& v) { /*if (&v != this) {}*/ if (_h) this->reset(); _h = v._h; v._h = 0; return *this; }
    // after this operation, v will be moved and becomes null
    Json& operator=(Json& v) { return this->operator=(std::move(v)); }
    // make a duplicate 
    Json dup() const { Json r; r._h = (_H*)this->_dup(); return r; }
    Json(f64& v): _h(new(xx::alloc()) _H(v)) {} Json(f64&& v): _h(new(xx::alloc()) _H(std::move(v))) {}
    Json(i64& v): _h(new(xx::alloc()) _H(v)) {} Json(i64&& v): _h(new(xx::alloc()) _H(std::move(v))) {}
    Json(long& v): Json(static_cast<i64>(v)) {} Json(long&& v): Json(std::move(static_cast<i64>(v))) {}
    Json(i16 v): Json(static_cast<i64>(v)) {} Json(i8 v): Json(static_cast<i64>(v)) {}
    Json(u64& v): _h(new(xx::alloc()) _H(v)) {} Json(u64&& v): _h(new(xx::alloc()) _H(std::move(v))) {}
    Json(i32 v): Json(static_cast<i64>(v)) {} Json(u32 v): Json(static_cast<u64>(v)) {}
    Json(long unsigned& v): Json(static_cast<u64>(v)) {} Json(long unsigned&& v): Json(std::move(static_cast<u64>(v))) {}
    Json(u16 v): Json(static_cast<u64>(v)) {} Json(u8 v): Json(static_cast<u64>(v)) {} Json(bool v): _h(new(xx::alloc()) _H(v)) {}
    // for string type
    Json(const void* p, size_t n): _h(new(xx::alloc()) _H(p, n)) {} Json(const char* s): Json(s, strlen(s)) {}
    Json(const std::string& s): Json(s.data(), s.size()) {} template<u16 I> Json(const text<I>& s) : Json(s.data(), s.size()) {}
    Json(const tm& _v) {
#ifdef _WIN32
      std::ostringstream os; os << std::setfill('0'); os << std::setw(4) << _v.tm_year + 1900;
#else
      std::ostringstream os; os << std::setfill('0'); int y = _v.tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
      os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
        << _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec;
      std::string s = os.str(); _h = new(xx::alloc()) _H(s.data(), s.size());
    }
    template<typename T, size_t N>
    Json(const std::array<T, N>& v): _h(new(xx::alloc()) _H(_arr_t())) { this->push_back(v[0]); for (u32 i = 1; i < N; ++i) this->push_back(v[i]); }
    template<typename T>
    Json(const std::stack<T>& v) : _h(new(xx::alloc()) _H(_arr_t())) { std::stack<T> $ = v; while (!$.empty())this->push_back($.top()), $.pop(); }
    template<typename T, std::enable_if_t<std::is_arr<T>::value || std::is_set<T>::value>* = null>
    Json(const T & v) : _h(new(xx::alloc()) _H(_arr_t())) {
      auto t = v.begin(); if (t != v.end()) { this->push_back(*t); while (++t != v.end()) this->push_back(*t); }
    }
    template<typename T, std::enable_if_t<std::is_mmp<std::remove_const_t<T>>::value>* = null>
    Json(std::remove_const_t<T>& v): _h(new(xx::alloc()) _H(_obj_t())) {
      Json j; std::string b; b.reserve(0xe0); b << '{'; auto t = v.begin(); if (t != v.end()) {
        j = &t->second; b << '"' << t->first << '"' << ':' << j.str();
        while (++t != v.end()) { j = &t->second; b << ',' << '"' << t->first << '"' << ':' << j.str(); }
      }
      b << '}'; this->parse_from(b.data(), b.size());
    }
    Json(_obj_t&& $): _h(new(xx::alloc()) _H(std::move($))) {} Json(_arr_t&& $): _h(new(xx::alloc()) _H(std::move($))) {}
    // make Json from initializer_list
    Json(std::initializer_list<Json> v); _FORCE_INLINE bool is_null() const { return _h == 0; } _FORCE_INLINE bool is_bool() const { return _h && (!_h->type); }
    _FORCE_INLINE bool is_int() const { return _h && (_h->type & t_int); } _FORCE_INLINE bool is_uint() const { return _h && (_h->type & t_uint); }
    _FORCE_INLINE bool is_double() const { return _h && (_h->type & t_double); } _FORCE_INLINE bool is_string() const { return _h && (_h->type & t_string); }
    _FORCE_INLINE bool is_array() const { return _h && (_h->type & t_array); } _FORCE_INLINE bool is_object() const { return _h && (_h->type & t_object); }
    // try to get a bool value
    //   - int or f64 type, 0 -> false, !0 -> true
    //   - string type, "true" or "1" -> true, otherwise -> false
    //   - other non-bool types, -> false
    bool as_bool() const {
      if (_h) {
        switch (_h->type) {
        case t_bool:   return _h->b;
        case t_int:    return _h->i != 0;
        case t_uint:   return _h->i != 0;
        case t_string: return std::lexical_cast<bool>(_h->s);
        case t_double: return _h->d != 0;
        }
      }
      return false;
    }
    // try to get an integer value
    //   - string or f64 type, convert to integer
    //   - bool type, true -> 1, false -> 0
    //   - other non-int types, -> 0
    i64 as_int64() const {
      if (_h) {
        switch (_h->type) {
        case t_int: case t_uint: return _h->i;
        case t_string: return std::lexical_cast<i64>(_h->s);
        case t_double: return static_cast<i64>(_h->d);
        case t_bool:   return _h->b ? 1 : 0;
        }
      }
      return 0;
    }
    u64 as_uint64() const {
      if (_h) {
        switch (_h->type) {
        case t_int: case t_uint: return static_cast<u64>(_h->i);
        case t_string: return std::lexical_cast<u64>(_h->s);
        case t_double: return static_cast<u64>(_h->d);
        case t_bool:   return _h->b ? 1 : 0;
        }
      }
      return 0;
    }
    int as_int() const {
      if (_h) {
        switch (_h->type) {
        case t_int: case t_uint: return static_cast<i32>(_h->i);
        case t_string: return std::lexical_cast<i32>(_h->s);
        case t_double: return static_cast<i32>(_h->d);
        case t_bool:   return _h->b ? 1 : 0;
        }
      }
      return 0;
    }
    u32 as_uint() const {
      if (_h) {
        switch (_h->type) {
        case t_int: case t_uint: return static_cast<u32>(_h->i);
        case t_string: return std::lexical_cast<u32>(_h->s);
        case t_double: return static_cast<u32>(_h->d);
        case t_bool:   return _h->b ? 1 : 0;
        }
      }
      return 0;
    }
    // try to get a f64 value
    //   - string or integer type, convert to f64
    //   - bool type, true -> 1, false -> 0
    //   - other non-f64 types, -> 0
    f64 as_double() const {
      if (_h) {
        switch (_h->type) {
        case t_double: return _h->d;
        case t_int:    return static_cast<f64>(_h->i);
        case t_uint:   return static_cast<f64>(static_cast<u64>(_h->i));
        case t_string: return std::lexical_cast<double>(_h->s);
        case t_bool:   return _h->b ? 1 : 0;
        }
      }
      return 0;
    }
    // returns a std::string.
    // for non-string types, it is equal to Json::str().
    //std::string as_string() const { return this->is_string() ? std::string(_h->s, _h->size) : this->str(); }
    std::string as_string() const {
      if (_h) {
        switch (_h->type) {
        case t_double: { char s[16]; return std::string(s, milo::dtoa(_h->d, s, 0x10)); }
        case t_int: { char b[20]; return std::string(b, i64toa(b, _h->i) - b); }
        case t_uint: { char b[20]; return std::string(b, u64toa(b, static_cast<u64>(_h->i)) - b); }
        case t_string: return std::string(_h->s, _h->size);
        case t_bool: return _h->b ? std::string("true", 4) : std::string("false", 5);
        }
      }
      return RES_NULL;
    }
    // Get a value of a template type like modern json
    template<typename T> _FORCE_INLINE T get() { T t{}; this->get_to(t); return t; }
    // get Json by index or key.
    //   - It is a read-only operation.
    //   - If the index is not in a valid range or the key does not exist, 
    //     the return value is a _reference to a null object.
    Json& get(u32 i) const; _FORCE_INLINE Json& get(int i) const { return this->get(static_cast<u32>(i)); }
    Json& get(const char* key) const;
    template <class T, class ...X> inline Json& get(T&& v, X&& ... x) const {
      Json& r = this->get(std::forward<T>(v)); return r.is_null() ? r : r.get(std::forward<X>(x)...);
    }
    template <typename T> _FORCE_INLINE void get_to(box<T>& $) {
      if (this->str() == RES_NULL) return; if (!$) $ = new T{}, * ((bool*)(&$)) = true; this->get_to(static_cast<T&>(*$));
    }
    template <typename T, std::enable_if_t<std::is_mmp<std::remove_const_t<T>>::value>* = null>
    _FORCE_INLINE void get_to(std::remove_const_t<T>& v) {
      if (!v.empty()) v.clear(); typename T::key_type _; typename T::mapped_type $;
      auto t = this->begin(); if (t != iterator::end()) {
        _ = std::lexical_cast<typename T::key_type>(t.key()); t.value().get_to($); v->emplace(std::make_pair(_, $));
        while (++t != iterator::end()) { _ = std::lexical_cast<typename T::key_type>(t.key()); t.value().get_to($); v->emplace(std::make_pair(_, $)); }
      }
    }
    template <typename T, size_t N>
    _FORCE_INLINE void get_to(std::array<T, N>& $) {
      T t; for (u32 i = 0; i < N; ++i) { this->get(i).get_to(t); $[i] = i < this->array_size() ? t : T{}; }
    }
    template <typename T>
    _FORCE_INLINE void get_to(std::forward_list<T>& $) {
      if (!$.empty()) $.clear(); for (u32 i = this->array_size(); --i != UINT32_MAX;) { T _{}; this->get(i).get_to(_); $.emplace_front(_); }
    }
    template <typename T>
    _FORCE_INLINE void get_to(std::stack<T>& $) {
      while (!$.empty())$.pop(); for (u32 i = this->array_size(); --i != UINT32_MAX;) { T _{}; this->get(i).get_to(_); $.emplace(_); }
    }
    template <typename T, std::enable_if_t<std::is_set<T>::value>* = null>
    _FORCE_INLINE void get_to(T& $) {
      if (!$.empty()) $.clear(); for (u32 i = 0; i < this->array_size(); ++i) { std::set_pack_t<T> _{}; this->get(i).get_to(_); $.emplace(_); }
    }
    template <typename T, std::enable_if_t<std::is_arr<T>::value>* = null>
    _FORCE_INLINE void get_to(T& $) {
      if (!$.empty()) $.clear(); for (u32 i = 0; i < this->array_size(); ++i) { std::arr_pack_t<T> _{}; this->get(i).get_to(_); $.emplace_back(_); }
    }
    //i8 i = -1; fc::ForEachField(fc::Tuple<T>(), &$, [&i, this](auto& t) { this->operator[](T::$[++i]).get_to(t); });
    template <typename T, std::enable_if_t<std::is_reg<T>::value>* = null>
    _FORCE_INLINE void get_to(T& $) {
      if (!_h) { _h = new(xx::alloc()) _H(_obj_t()); return; } if (_h->type == t_object) {
        std::Tuple<T>::from_json(*this, &$);
        //i8 i = -1; std::Tuple<T>::For(&$, [&i, this](auto& t) { this->operator[](std::Tuple<T>::$[++i]).get_to(t); });
      }
    }
    _FORCE_INLINE void get_to(bool& $) { $ = this->as_bool(); } _FORCE_INLINE void get_to(f32& $) { $ = static_cast<f32>(this->as_double()); }
    _FORCE_INLINE void get_to(f64& $) { $ = this->as_double(); } _FORCE_INLINE void get_to(i8& $) { $ = static_cast<i8>(this->as_int64()); }
    _FORCE_INLINE void get_to(i16& $) { $ = static_cast<i16>(this->as_int64()); } _FORCE_INLINE void get_to(i32& $) { $ = this->as_int(); }
    _FORCE_INLINE void get_to(i64& $) { $ = this->as_int64(); } _FORCE_INLINE void get_to(u8& $) { $ = static_cast<u8>(this->as_uint64()); }
    _FORCE_INLINE void get_to(u16& $) { $ = static_cast<u16>(this->as_uint64()); } _FORCE_INLINE void get_to(u32& $) { $ = this->as_uint(); }
    _FORCE_INLINE void get_to(u64& $) { $ = this->as_uint64(); }
    _FORCE_INLINE void get_to(std::string& $) { $ = this->is_string() ? std::string(_h->s, _h->size) : ""; }
    template <u16 I> _FORCE_INLINE void get_to(text<I>& $) { $ = this->is_string() ? std::string(_h->s, _h->size) : ""; }
    _FORCE_INLINE void get_to(tm& $) {
      const char* c = this->is_string() ? _h->s : "0000-00-00 00:00:00"; int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
      if (sscanf(c, "%4d-%2d-%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min, &sec) == 6)
        $.tm_mday = day, $.tm_hour = hour, $.tm_min = min, $.tm_sec = sec; $.tm_year = year - 1900; $.tm_mon = month - 1;
    }
    _FORCE_INLINE void operator=(const char* v) { *this = Json(v, strlen(v)); }
    template <u16 I> _FORCE_INLINE void operator=(const text<I>& s) { *this = Json(s.data(), s.size()); }
    _FORCE_INLINE void operator=(const std::string& s) { *this = Json(s.data(), s.size()); }
    void operator=(const tm& _v) {
      this->reset(); std::ostringstream os; os << std::setfill('0');
#ifdef _WIN32
      os << std::setw(4) << _v.tm_year + 1900;
#else
      int y = _v.tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
#endif
      os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
        << _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec;
      std::string s = os.str(); _h = new(xx::alloc()) _H(s.data(), s.size());
    }
    template <typename T, std::enable_if_t<std::is_fundamental<T>::value>* = null>
    _FORCE_INLINE void operator=(const T& s) { *this = Json(const_cast<T&>(s)); }
    _FORCE_INLINE void operator=(std::nullptr_t) { *this = json::RES_NULL; }
    template <typename T>
    _FORCE_INLINE void operator=(const box<T>& v) { if (v) *this = *v; }
    //i8 i = -1; fc::ForEachField(fc::Tuple<T>(), &v, [&i, this](auto& t) { this->operator[](T::$[++i]) = t; });
    template <typename T, std::enable_if_t<std::is_reg<T>::value>* = null>
    _FORCE_INLINE void operator=(const T& v) {
      std::Tuple<T>::to_json(*this, const_cast<T*>(&v));
      //i8 i = -1; std::Tuple<T>::For(const_cast<T*>(&v), [&i, this](auto& t) { this->operator[](std::Tuple<T>::$[++i]) = t; });
    }
    template <typename T, size_t N>
    _FORCE_INLINE void operator=(const std::array<T, N>& $) {
      *this = Json(Json::_arr_t()); Json j; for (u32 i = 0; i < N; ++i) { j = $[i]; this->push_back(j); }
    }
    template <typename T>
    _FORCE_INLINE void operator=(const std::stack<T>& v) {
      *this = Json(Json::_arr_t()); if (!v.empty()) {
        std::stack<T> t = v; Json j; this->push_back(t.top()); t.pop(); while (!t.empty()) { this->push_back(t.top()); t.pop(); }
      }
    }
    template <typename T, std::enable_if_t<std::is_set<T>::value>* = null>
    _FORCE_INLINE void operator=(const T& v) {
      *this = Json(Json::_arr_t()); if (!v.empty()) { Json j; for (const std::set_pack_t<T>& t : v) { j = t; this->push_back(j); } }
    }
    template <typename T, std::enable_if_t<std::is_arr<T>::value>* = null>
    _FORCE_INLINE void operator=(const T& v) {
      *this = Json(Json::_arr_t()); if (!v.empty()) { Json j; for (const std::arr_pack_t<T>& t : v) { j = t; this->push_back(j); } }
    }
    template <typename T, std::enable_if_t<std::is_mmp<T>::value>* = null>
    _FORCE_INLINE void operator=(const T& v) {
      *this = Json(Json::_obj_t()); if (!v.empty()) {
        std::string b(0xe0, 0); for (auto& t : v) { b.clear(); b << t.first; this->add_member(b.c_str(), t.second); }
      }
    }
    // set value for Json.
    //   - The last parameter is the value, other parameters are index or key.
    //   - eg.
    //     Json x;
    //     x.set("a", "b", 0, 3);  // x-> {"a": {"b": [3]}}
    template <class T> _FORCE_INLINE Json& set(T&& v) { return *this = Json(std::forward<T>(v)); }
    template <class A, class B, class ...X>
    _FORCE_INLINE Json& set(A&& a, B&& b, X&& ... x) {
      Json& r = this->_set(std::forward<A>(a)); return r.set(std::forward<B>(b), std::forward<X>(x)...);
    }
    // push v to an array.
    // if the Json calling this method is not an array, it will be reset to an array.
    Json& push_back(Json&& v) {
      if (_h && (_h->type & t_array)) {
        if (_unlikely(!_h->p)) new(&_h->p) xx::Array(8); _array().push_back(v._h); v._h = 0; return *this;
      } this->reset(); _h = new(xx::alloc()) _H(_arr_t()); new(&_h->p) xx::Array(8); _array().push_back(v._h); v._h = 0; return *this;
    }
    _FORCE_INLINE Json& push_back(Json& v) { return this->push_back(std::move(v)); }
    // remove the ith element from an array
    // the last element will be moved to the ith place
    void remove(u32 i) { if (this->is_array() && i < this->array_size()) { ((Json&)_array()[i]).reset(); _array().remove(i); } }
    _FORCE_INLINE void remove(int i) { this->remove((u32)i); }
    void remove(const char* key);
    // erase the ith element from an array
    void erase(u32 i) { if (this->is_array() && i < this->array_size()) { ((Json&)_array()[i]).reset(); _array().erase(i); } }
    _FORCE_INLINE void erase(int i) { this->erase((u32)i); }
    void erase(const char* key);
    // it is better to use get() instead of this method.
    _FORCE_INLINE Json& operator[](u32 i) const { return (Json&)_array()[i]; }
    _FORCE_INLINE Json& operator[](int i) const { return this->operator[](static_cast<u32>(i)); }
    bool operator==(bool v) const { return this->is_bool() && _h->b == v; } bool operator==(f64 v) const { return this->is_double() && _h->d == v; }
    bool operator==(i64 v) const { return (this->is_int() || this->is_uint()) && _h->i == v; }
    bool operator==(u64 v) const { return (this->is_int() || this->is_uint()) && static_cast<u64>(_h->i) == v; }
    bool operator==(int v) const { return this->operator==(static_cast<i64>(v)); }
    bool operator==(u32 v) const { return this->operator==(static_cast<u64>(v)); }
    bool operator==(long v) const { return this->operator==(static_cast<i64>(v)); }
    bool operator==(long unsigned v) const { return this->operator==(static_cast<u64>(v)); }
    bool operator==(const char* v) const { return this->is_string() && strcmp(_h->s, v) == 0; }
    bool operator==(const std::string& v) const { return this->is_string() && v == _h->s; }
    bool operator!=(bool v) const { return !this->operator==(v); } bool operator!=(f64 v) const { return !this->operator==(v); }
    bool operator!=(i64 v) const { return !this->operator==(v); } bool operator!=(u64 v) const { return !this->operator==(v); }
    bool operator!=(int v) const { return !this->operator==(v); } bool operator!=(u32 v) const { return !this->operator==(v); }
    bool operator!=(long v) const { return !this->operator==(v); } bool operator!=(long unsigned v) const { return !this->operator==(v); }
    bool operator!=(const char* v) const { return !this->operator==(v); }
    bool operator!=(const std::string& v) const { return !this->operator==(v); }
    // for array and object, return number of the elements.
    // for string, return the length.
    // for other types, return 0.
    u32 size() const {
      if (_h) {
        switch (_h->type) {
        case t_array: return _h->p ? _array().size() : 0;
        case t_object: return _h->p ? (_array().size() >> 1) : 0;
        case t_string: return _h->size;
        }
      }
      return 0;
    }
    _FORCE_INLINE u32 type() const { return _h ? _h->type : 0u; }
    _FORCE_INLINE bool empty() const { return this->size() == 0; }
    _FORCE_INLINE u32 array_size() const { if (_h && (_h->type & t_array)) { return _h->p ? _array().size() : 0; } return 0; }
    _FORCE_INLINE u32 object_size() const { if (_h && (_h->type & t_object)) { return _h->p ? (_array().size() >> 1) : 0; } return 0; }
    _FORCE_INLINE u32 string_size() const { return (_h && (_h->type & t_string)) ? _h->size : 0; }
    // push key-value to the back of an object, key may be repeated.
    // if the Json calling this method is not an object, it will be reset to an object.
    Json& add_member(const char* key, Json&& v) {
      if (_h && (_h->type & t_object)) {
        if (_unlikely(!_h->p)) new(&_h->p) xx::Array(16);
      } else {
        this->reset(); _h = new(xx::alloc()) _H(_obj_t()); new(&_h->p) xx::Array(16);
      }
      _array().push_back(xx::alloc_string(key, strlen(key))); // key
      _array().push_back(v._h); v._h = 0; return *this;
    }
    _FORCE_INLINE Json& add_member(const char* key, Json& v) { this->add_member(key, std::move(v)); return *this; }
    bool has_member(const char* key) const;
    // it is better to use get(key) instead of this method.
    Json& operator[](const char* key) const;
    Json& operator[](const std::string_view& key) const;
    class iterator {
      void** _p, * _e; u32 _step;
    public:
      iterator(void** p, void** e, u32 step): _p(p), _e(e), _step(step) {} struct End {}; // fake end
      static const End& end() { static End kEnd; return kEnd; } _FORCE_INLINE iterator& operator++() { _p += _step; return *this; }
      iterator operator++(int) = delete; _FORCE_INLINE const char*& key() const { return (const char*&)_p[0]; }
      _FORCE_INLINE bool operator!=(const End&) const { return _p != _e; }
      _FORCE_INLINE bool operator==(const End&) const { return _p == _e; }
      _FORCE_INLINE Json& value() const { return reinterpret_cast<Json&>(_p[1]); }
      _FORCE_INLINE Json& operator*() const { return reinterpret_cast<Json&>(_p[0]); }
    };
    _FORCE_INLINE u32 count(const std::string& key) const {
      for (iterator it = this->begin(); it != this->end(); ++it) { if (it.key() == key) { return 1; } } return 0;
    }
    // the begin iterator.
    //   - If Json is not array or object type, the return value is equal to the end iterator.
    iterator begin() const {
      if (_h && _h->p && (_h->type & (t_array | t_object))) {
        xx::Array& a = _array(); return iterator(a.data(), a.data() + a.size(), _h->type >> 4);
      }
      return iterator(0, 0, 0);
    }
    // a fake end iterator
    _FORCE_INLINE const iterator::End& end() const { return iterator::end(); }
    // Stringify.
    //   - str() converts Json to minified string.
    //   - dump() converts Json to human readable string with indent.
    //   - mdp: max decimal places for float point numbers.
    std::string& str(std::string& fs, int mdp = 16) const;
    _FORCE_INLINE std::string& dump(std::string& s, int indent = 4, int mdp = 16) const { return this->_json2pretty(s, indent, indent, mdp); }
    _FORCE_INLINE std::string str(int mdp = 16) const { std::string s(0xe0, 0); s.clear(); return this->str(s, mdp); }
    _FORCE_INLINE std::string dump(int i = 2, int m = 16) const { std::string s(0xe0, 0); s.clear(); return this->_json2pretty(s, i, i, m); }
    _FORCE_INLINE std::string pretty(int indent = 4, int mdp = 16) const { std::string s; s.reserve(0xe0); this->dump(s, indent, mdp); return s; }
    // Parse Json from string, inverse to stringify.
    bool parse_from(const char* s, size_t n); _FORCE_INLINE bool parse_from(const char* s) { return this->parse_from(s, strlen(s)); } void reset();
    _FORCE_INLINE bool parse_from(const std::string& s) { return this->parse_from(s.data(), s.size()); } bool parse(const char* s, size_t n);
    _FORCE_INLINE void swap(Json& v) noexcept { Json::_H* h = _h; _h = v._h; v._h = h; }_FORCE_INLINE void swap(Json&& v) noexcept { v.swap(*this); }
  private:
    friend class Parser; _H* _h; void* _dup() const; _FORCE_INLINE xx::Array& _array() const { return (xx::Array&)_h->p; }
    Json& _set(u32 i); _FORCE_INLINE Json& _set(int i) { return this->_set(static_cast<u32>(i)); } Json& _set(const char* key);
    std::string& _json2pretty(std::string& fs, int indent, int n, int mdp) const;
  };
  // make an empty array
  _FORCE_INLINE Json array() { return Json(Json::_arr_t()); }
  // make an array from initializer_list
  Json array(std::initializer_list<Json> v);
  // make an empty object
  _FORCE_INLINE Json object() { return Json(Json::_obj_t()); }
  // make an object from initializer_list
  Json object(std::initializer_list<Json> v);
  //Quick parsing without annotations
  _FORCE_INLINE Json parse(const char* s, size_t n) { Json r; r.parse_from(s, n); return r; }
  //Average parsing without comments
  _FORCE_INLINE Json parse(const char* s) { Json r; r.parse_from(s, strlen(s)); return r; }
  //Slow parsing with comments
  _FORCE_INLINE Json parse(const std::string& s) { Json r; r.parse(s.data(), s.size()); return r; }
  static Json read_file(const char* p) {
    struct stat t; if (-1 == stat(p, &t)) throw std::runtime_error("File not exist.");
    if (t.st_size > 10485760) throw std::runtime_error("Can't be greater than 10M.");
    std::ifstream f(p, std::ios::in | std::ios::binary); if (!f.is_open() || f.fail()) throw std::runtime_error("Failed to open!");
    char* s = new char[t.st_size]; f.read(s, t.st_size); Json r; r.parse(s, t.st_size); delete[] s; return r;
  }
  static const json::Json empty_str("", 1), nullContext;
} // json
typedef json::Json Json; _FORCE_INLINE std::string& operator<<(std::string& fs, const json::Json& x) { return x.str(fs); }
template<typename T, std::enable_if_t<std::is_reg<T>::value>* = null>
_FORCE_INLINE std::string& operator<<(std::string& b, const box<T>& _v) {
  if (_v) { Json j; std::Tuple<T>::to_json(j, const_cast<T*>(_v.operator->())); return b << j.str(); } return b.append("null", 4);
};
template<typename T, std::enable_if_t<std::is_reg<T>::value>* = null>
_FORCE_INLINE std::string& operator<<(std::string& b, const T& _v) { Json j; std::Tuple<T>::to_json(j, const_cast<T*>(&_v)); return b << j.str(); };
template<typename T, std::enable_if_t<std::is_reg<T>::value>* = null>
_FORCE_INLINE std::ostream& operator<<(std::ostream& os, T& v) { std::string b; b << v; return os.write(b.data(), b.size()); }
namespace fc {
  template<typename T> static void to_json(json::Json& c, const box<T>* v) { if (c.size())c.reset(); if (*v) c = **v; else c = null; }
  template<typename T, std::enable_if_t<std::is_arr<T>::value>* = null>
  static void to_json(json::Json& c, T* v) {
    using K = std::arr_pack_t<T>; if (c.size()) c = Json(Json::_arr_t()); Json j; auto t = v->begin(); if (t != v->end()) {
      j = static_cast<K>(*t); c.push_back(j); while (++t != v->end())j = static_cast<K>(*t), c.push_back(j);
    }
  }
  template<typename T, std::enable_if_t<std::is_mmp<T>::value && !std::is_reg<typename T::key_type>::value>* = null>
  static void to_json(json::Json& c, T* v) {
    std::string b; b.reserve(0xe0); if (c.size()) c = Json(Json::_obj_t()); auto t = v->begin(); if (t != v->end()) {
      b << t->first; c.add_member(b.c_str(), t->second); while (++t != v->end()) { b.clear(); b << t->first; c.add_member(b.c_str(), t->second); }
    }
  }
  template<typename T, size_t N>
  static void to_json(json::Json& c, std::array<T, N>* v) {
    if (c.size()) c = Json(Json::_arr_t()); auto t = v->begin(); if (t != v->end()) { c.push_back(*t); while (++t != v->end())c.push_back(*t); }
  }
  template<typename T, std::enable_if_t<!std::is_arr<T>::value && !std::is_mmp<T>::value>* = null>
  static void to_json(json::Json& c, T* v) { if (c.size())c.reset(); if (v) c = *v; else c = null; }
  template<typename T, size_t N>
  static void from_json(const json::Json& c, std::array<T, N>* v) {
    T t; for (u32 i = 0; i < N; ++i) { c.get(i).get_to(t); v->operator[](i) = i < c.array_size() ? t : T{}; }
  }
  template<typename T>
  static void from_json(const json::Json& c, std::forward_list<T>* v) {
    T t; if (!v->empty())v->clear(); for (u32 i = c.array_size(); --i != UINT32_MAX;) { c.get(i).get_to(t); v->emplace_front(t); }
  }
  template<typename T, std::enable_if_t<std::is_mmp<std::remove_reference_t<T>>::value>* = null>
  static void from_json(const json::Json& c, T* v) {
    typename T::key_type _; typename T::mapped_type $; if (!v->empty())v->clear();
    auto t = c.begin(); if (t != Json::iterator::end()) {
      _ = std::lexical_cast<typename T::key_type>(t.key()); t.value().get_to($); v->emplace(std::make_pair(_, $));
      while (++t != Json::iterator::end()) { _ = std::lexical_cast<typename T::key_type>(t.key()); t.value().get_to($); v->emplace(std::make_pair(_, $)); }
    }
  }
  template <typename T>
  static void from_json(const json::Json& c, std::stack<T>* v) {
    T t; while (!v->empty())v->pop(); for (u32 i = c.array_size(); --i != UINT32_MAX;) { c.get(i).get_to(t); v->emplace(t); }
  }
  template<typename T, std::enable_if_t<std::is_set<std::remove_reference_t<T>>::value>* = null>
  static void from_json(const json::Json& c, T* v) {
    std::set_pack_t<T> t; if (!v->empty())v->clear(); for (u32 i = 0; i < c.array_size(); ++i) { c.get(i).get_to(t); v->emplace(t); }
  }
  template<typename T, std::enable_if_t<std::is_arr<std::remove_reference_t<T>>::value>* = null>
  static void from_json(const json::Json& c, T* v) {
    std::arr_pack_t<T> t; if (!v->empty())v->clear(); for (u32 i = 0; i < c.array_size(); ++i) { c.get(i).get_to(t); v->emplace_back(t); }
  }
  //Can use modern json instead of from_json method, use: ... = c.get<T>();
  template<typename T, std::enable_if_t<std::is_reg<T>::value>* = null>
  static void from_json(const json::Json& c, T* _) { if (_)const_cast<json::Json&>(c).get_to(*_); }
}

#if !defined(__cplusplus) || (__cplusplus <= 201103L)
#define _FC_F_T_ _FORCE_INLINE const std::string_view
#define _FC_F_T const char*
#define _FC_NAME(_VA_ARGS__) _VA_ARGS__
#define _FC_ATTR(_VA_ARGS__,__VA_ARGS_) "`"#_VA_ARGS__"`.`"#__VA_ARGS_"`"
#define _FC_FIELD(__VA_ARGS_) #__VA_ARGS_
#define _FC_END_HOLDER ""
template <typename F, typename K> _FC_F_T_ k(K F::* c) { return F::$[F::_idex(c)]; };
#elif (__cplusplus == 201402L)
#define _FC_F_T_ constexpr const std::string_view
#define _FC_F_T const char*
#define _FC_NAME(_VA_ARGS__) _VA_ARGS__
#define _FC_LEN(_VA_ARGS__,__VA_ARGS_) sizeof("`"#_VA_ARGS__"`.`"#__VA_ARGS_"`") - 1
#define _FC_ATTR(_VA_ARGS__,__VA_ARGS_) "`"#_VA_ARGS__"`.`"#__VA_ARGS_"`"
#define _FC_FIELD(__VA_ARGS_) #__VA_ARGS_
#define _FC_END_HOLDER ""
template <typename F> _FC_F_T_ RESk(int T) { return std::string_view(F::$[T], F::L[T]); };
template <typename F, typename K> _FC_F_T_ k(K F::* c) { return RESk<F>(F::_idex(c)); };
#else
#define _FC_F_T_ constexpr const std::string_view&
#define _FC_F_T const std::string_view
#define _FC_NAME(_VA_ARGS__) std::string_view(_VA_ARGS__,sizeof(_VA_ARGS__)-1)
#define _FC_ATTR(_VA_ARGS__,__VA_ARGS_) _FC_NAME("`"#_VA_ARGS__"`.`"#__VA_ARGS_"`")
#define _FC_FIELD(__VA_ARGS_) _FC_NAME(#__VA_ARGS_)
#define _FC_END_HOLDER std::string_view("",1)
template <typename F, typename K> _FC_F_T_ k(K F::* c) { return F::$[F::_idex(c)]; };
#endif
//#define _RR1(N,...) _EXP(_R_##N(RPK, __VA_ARGS__))
//#define _RR(N,...) _RR1(N,__VA_ARGS__)
#define _FC_TO(__VA_ARGS_) c[#__VA_ARGS_].operator= (_->__VA_ARGS_)
#define _FC_FROM(__VA_ARGS_) c.get(#__VA_ARGS_).get_to(_->__VA_ARGS_)
#define _FC_PTR(_VA_ARGS__,__VA_ARGS_) &_VA_ARGS__::__VA_ARGS_
#define _FC_TYPE(_VA_ARGS__,__VA_ARGS_) decltype(&_VA_ARGS__::__VA_ARGS_)
#define _FC_TAG(__VA_ARGS_,_VA_ARGS__)fc::_ref(&__VA_ARGS_::_VA_ARGS__,e)
#define _FC_OFSET(__VA_ARGS_,_VA_ARGS__) reinterpret_cast<size_t>(&(((__VA_ARGS_*)0)->*&__VA_ARGS_::_VA_ARGS__))
#define REG using _$_=std::in_place_t;private:static const std::string _name;
//sql key check type
enum TC { NIL, NOT_NULL, PRIMARY_KEY, AUTO_INCREMENT = 4, UNIQUIE = 8, INDEX = 16 };
//Registering a primary key will be used in conjunction with subsequent methods. But in this case, the first key must be the primary key
#define RFK(_,...) _JVM(_); static constexpr const TC _key[] = { __VA_ARGS__ };
//Register each key in order, including the primary key, but use it at the end of the structure, that is, after the macro RAT
#define RKS(...) static constexpr const TC _key[_s] = { __VA_ARGS__ };
#if (__cplusplus == 201402L)
#define RAT(__VA_ARGS_,...)using _$_=std::in_place_t;template<typename F>friend _FC_F_T_ RESk(int T);template<typename F,typename K>friend _FC_F_T_ k(K F::* c);\
const constexpr static u32 _s=NUM_ARGS(__VA_ARGS__);private:constexpr static u32 L[]{_EXP(_M_$(_FC_LEN,__VA_ARGS_,__VA_ARGS__)), 0};\
constexpr static _FC_F_T $[]{ _EXP(_M_$(_FC_ATTR,__VA_ARGS_,__VA_ARGS__)), _FC_END_HOLDER };static constexpr _FC_F_T _alias=_FC_NAME("`"#__VA_ARGS_"`");\
static const std::string _name;public:\
static constexpr std::tuple<_EXP(_M_$(_FC_TYPE,__VA_ARGS_,__VA_ARGS__))> __(){return std::make_tuple(_EXP(_M_$(_FC_PTR,__VA_ARGS_,__VA_ARGS__)));}\
template <typename E>constexpr static int _idex(const E& e) { return fc::count_first_falses(_EXP(_M_$(_FC_TAG,__VA_ARGS_,__VA_ARGS__))); }
#else
#define RAT(__VA_ARGS_,...)using _$_=std::in_place_t;template<typename F,typename K>friend _FC_F_T_ k(K F::* c);\
const constexpr static u32 _s=NUM_ARGS(__VA_ARGS__);private:static constexpr _FC_F_T _alias=_FC_NAME("`"#__VA_ARGS_"`");\
constexpr static _FC_F_T $[]{ _EXP(_M_$(_FC_ATTR,__VA_ARGS_,__VA_ARGS__)), _FC_END_HOLDER }; static const std::string _name;public:\
static constexpr std::tuple<_EXP(_M_$(_FC_TYPE,__VA_ARGS_,__VA_ARGS__))> __(){return std::make_tuple(_EXP(_M_$(_FC_PTR,__VA_ARGS_,__VA_ARGS__)));}\
template <typename E>constexpr static int _idex(const E& e) { return fc::count_first_falses(_EXP(_M_$(_FC_TAG,__VA_ARGS_,__VA_ARGS__))); }
#endif
//with table's _name
#define REGIS(__VA_ARGS_,...) const std::string __VA_ARGS_::_name=fc::toSqlCase(#__VA_ARGS_);template<>struct std::Tuple<__VA_ARGS_>{\
const constexpr static u32 _s=NUM_ARGS(__VA_ARGS__);constexpr static _FC_F_T $[]{ _EXP(_R$(_FC_FIELD,__VA_ARGS__)) };\
static const constexpr std::tuple<_EXP(_M_$(_FC_TYPE,__VA_ARGS_,__VA_ARGS__))> __(){return std::make_tuple(_EXP(_M_$(_FC_PTR,__VA_ARGS_,__VA_ARGS__)));}\
static void from_json(const Json& c,__VA_ARGS_* _) { _EXP(_R$(_FC_FROM,__VA_ARGS__)); }\
static void to_json(Json& c,__VA_ARGS_* _) { if (c.size())c.reset(); _EXP(_R$(_FC_TO,__VA_ARGS__)); }\
  };
#if __clang__
#pragma clang diagnostic pop
#endif
#endif