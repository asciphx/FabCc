#ifndef JSON_H
#define JSON_H
#ifdef _MSC_VER
#pragma warning (disable:4200)
#ifndef __thread
#define __thread __declspec(thread)
#endif
#else
#include <math.h>
#endif
#include "str.hh"
#include "buf.hh"
#include "h/macros.h"
#include "hpp/box.hpp"
#include "hpp/tuple.hpp"
#include <cassert>
#include <initializer_list>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <tuple>
// from https://github.com/idealvin/coost/blob/master/include/co/json.h
namespace json {
  static const std::string RES_NULL("null");
  namespace xx {
	class Array {
	public:
	  typedef void* T;
	  struct _H { u32 cap; u32 size; T p[]; };
	  static const size_t N = sizeof(T);
	  static const u32 R = sizeof(_H) / N;
	  explicit Array(u32 cap) {
		_h = (_H*) ::malloc(N * (R + cap)); _h->cap = cap; _h->size = 0;
	  }
	  Array(): Array(1024 - R) {}
	  ~Array() { ::free(_h); }
	  T* data() const { return _h->p; }
	  u32 size() const { return _h->size; }
	  bool empty() const { return this->size() == 0; }
	  void resize(u32 n) { _h->size = n; }
	  T& back() const { return _h->p[this->size() - 1]; }
	  T& operator[](u32 i) const { return _h->p[i]; }
	  void push_back(T v) {
		if (_h->size == _h->cap) {
		  const size_t n = N * _h->cap, o = sizeof(_H) + n;
		  _h = (_H*) ::realloc(_h, o + n); assert(_h); _h->cap <<= 1;
		}
		_h->p[_h->size++] = v;
	  }
	  void remove(u32 i) { if (i != --_h->size) _h->p[i] = _h->p[_h->size]; }
	  void remove_pair(u32 i) {
		if (i != (_h->size -= 2)) {
		  _h->p[i] = _h->p[_h->size]; _h->p[i + 1] = _h->p[_h->size + 1];
		}
	  }
	  T pop_back() { return _h->p[--_h->size]; }
	  void erase(u32 i) {
		if (i != --_h->size) {
		  memmove(_h->p + i, _h->p + i + 1, (_h->size - i) * N);
		}
	  }
	  void erase_pair(u32 i) {
		if (i != (_h->size -= 2)) {
		  memmove(_h->p + i, _h->p + i + 2, (_h->size - i) * N);
		}
	  }
	  void reset() {
		_h->size = 0; if (_h->cap > 8192) { ::free(_h); new(this) Array(); }
	  }
	private:
	  _H* _h;
	};
	void* alloc(); char* alloc_string(const void* p, size_t n);
  } // xx
  class Json {
  public:
	enum {
	  t_bool = 0, t_int = 1, t_uint = 2, t_double = 4,
	  t_string = 8, t_array = 16, t_object = 32
	};
	struct _obj_t {};
	struct _arr_t {};
	struct _H {
	  _H(bool v) noexcept: type(t_bool), b(v) {}
	  _H(i64 v) noexcept: type(t_int), i(v) {}
	  _H(u64 v) noexcept: type(t_uint), i(static_cast<i64>(v)) {}
	  _H(double v) noexcept: type(t_double), d(v) {}
	  _H(_obj_t) noexcept: type(t_object), p(0) {}
	  _H(_arr_t) noexcept: type(t_array), p(0) {}
	  _H(const char* p): _H(p, strlen(p)) {}
	  _H(const void* p, size_t n): type(t_string), size((u32)n) {
		s = xx::alloc_string(p, n);
	  }
	  u32 type;
	  u32 size;  // size of string
	  union {
		bool b;   // for bool
		i64 i;  // for number
		double d; // for double
		char* s;  // for string
		void* p;  // for array and object
	  };
	};
	Json() noexcept: _h(0) {}
	Json(decltype(nullptr)) noexcept: _h(0) {}
	Json(Json&& v) noexcept: _h(v._h) { v._h = 0; }
	Json(Json& v) noexcept: _h(v._h) { v._h = 0; }
	~Json() { if (_h) this->reset(); }
	Json(const Json& v) = delete;
	void operator=(const Json&) = delete;
	Json& operator=(Json&& v) {
	  if (&v != this) { if (_h) this->reset(); _h = v._h; v._h = 0; } return *this;
	}
	// after this operation, v will be moved and becomes null
	Json& operator=(Json& v) { return this->operator=(std::move(v)); }
	// make a duplicate 
	Json dup() const { Json r; r._h = (_H*)this->_dup(); return r; }
	Json(bool v): _h(new(xx::alloc()) _H(v)) {}
	Json(double v): _h(new(xx::alloc()) _H(v)) {}
	Json(i64 v): _h(new(xx::alloc()) _H(v)) {}
	Json(i32 v): Json((i64)v) {}
	Json(long v): Json((i64)v) {}
	Json(i16 v): Json((i64)v) {}
	Json(i8 v): Json((i64)v) {}
	Json(u64 v): _h(new(xx::alloc()) _H(v)) {}
	Json(u32 v): Json((u64)v) {}
	Json(long unsigned v): Json((u64)v) {}
	Json(u16 v): Json((u64)v) {}
	Json(u8 v): Json((u64)v) {}
	// for string type
	Json(const void* p, size_t n): _h(new(xx::alloc()) _H(p, n)) {}
	Json(const char* s): Json(s, strlen(s)) {}
	Json(const std::string& s): Json(s.data(), s.size()) {}
	Json(const tm& _v) {
	  std::ostringstream os; os << std::setfill('0');
  #ifdef _WIN32
	  os << std::setw(4) << _v.tm_year + 1900;
  #else
	  int y = _v.tm_year / 100; os << std::setw(2) << 19 + y << std::setw(2) << _v.tm_year - y * 100;
  #endif
	  os << '-' << std::setw(2) << (_v.tm_mon + 1) << '-' << std::setw(2) << _v.tm_mday << ' ' << std::setw(2)
		<< _v.tm_hour << ':' << std::setw(2) << _v.tm_min << ':' << std::setw(2) << _v.tm_sec;
	  std::string s = os.str(); _h = new(xx::alloc()) _H(s.data(), s.size());
	}
	Json(_obj_t): _h(new(xx::alloc()) _H(_obj_t())) {}
	Json(_arr_t): _h(new(xx::alloc()) _H(_arr_t())) {}
	// make Json from initializer_list
	Json(std::initializer_list<Json> v);
	bool is_null() const { return _h == 0; }
	bool is_bool() const { return _h && (!_h->type); }
	bool is_int() const { return _h && (_h->type & t_int); }
	bool is_uint() const { return _h && (_h->type & t_uint); }
	bool is_double() const { return _h && (_h->type & t_double); }
	bool is_string() const { return _h && (_h->type & t_string); }
	bool is_array() const { return _h && (_h->type & t_array); }
	bool is_object() const { return _h && (_h->type & t_object); }
	// try to get a bool value
	//   - int or double type, 0 -> false, !0 -> true
	//   - string type, "true" or "1" -> true, otherwise -> false
	//   - other non-bool types, -> false
	bool as_bool() const {
	  if (_h) {
		switch (_h->type) {
		case t_bool:   return _h->b;
		case t_int:    return _h->i != 0;
		case t_uint:   return _h->i != 0;
		case t_string: return str::to_bool(_h->s);
		case t_double: return _h->d != 0;
		}
	  }
	  return false;
	}
	// try to get an integer value
	//   - string or double type, convert to integer
	//   - bool type, true -> 1, false -> 0
	//   - other non-int types, -> 0
	i64 as_int64() const {
	  if (_h) {
		switch (_h->type) {
		case t_int:    return _h->i;
		case t_uint:   return static_cast<i64>((u64)_h->i);
		case t_string: return str::to_int64(_h->s);
		case t_double: return (i64)_h->d;
		case t_bool:   return _h->b ? 1 : 0;
		}
	  }
	  return 0;
	}
	u64 as_uint64() const {
	  if (_h) {
		switch (_h->type) {
		case t_int:    return static_cast<u64>(_h->i);
		case t_uint:   return static_cast<u64>(_h->i);
		case t_string: return str::to_uint64(_h->s);
		case t_double: return (u64)_h->d;
		case t_bool:   return _h->b ? 1 : 0;
		}
	  }
	  return 0;
	}
	int as_int() const {
	  if (_h) {
		switch (_h->type) {
		case t_int:    return static_cast<i32>(_h->i);
		case t_uint:   return static_cast<i32>((u64)_h->i);
		case t_string: return str::to_int32(_h->s);
		case t_double: return (i32)_h->d;
		case t_bool:   return _h->b ? 1 : 0;
		}
	  }
	  return 0;
	}
	u32 as_uint() const {
	  if (_h) {
		switch (_h->type) {
		case t_int:    return static_cast<u32>(_h->i);
		case t_uint:   return static_cast<u32>((u64)_h->i);
		case t_string: return str::to_uint32(_h->s);
		case t_double: return (u32)_h->d;
		case t_bool:   return _h->b ? 1 : 0;
		}
	  }
	  return 0;
	}
	// try to get a double value
	//   - string or integer type, convert to double
	//   - bool type, true -> 1, false -> 0
	//   - other non-double types, -> 0
	double as_double() const {
	  if (_h) {
		switch (_h->type) {
		case t_double: return _h->d;
		case t_int:
		case t_uint:   return (double)_h->i;
		case t_string: return str::to_double(_h->s);
		case t_bool:   return _h->b ? 1 : 0;
		}
	  }
	  return 0;
	}
	// returns a c-style string, null-terminated.
	// for non-string types, returns an empty string.
	const char* as_c_str() const { return this->is_string() ? _h->s : ""; }
	// returns a std::string.
	// for non-string types, it is equal to Json::str().
	fc::Buf as_string() const { return this->is_string() ? fc::Buf(_h->s, _h->size) : this->str(); }
	// get Json by index or key.
	//   - It is a read-only operation.
	//   - If the index is not in a valid range or the key does not exist, 
	//     the return value is a _reference to a null object.
	Json& get() const { return *(Json*)this; }
	Json& get(u32 i) const;
	Json& get(int i) const { return this->get((u32)i); }
	Json& get(const char* key) const;
	template <class T, class ...X>
	inline Json& get(T&& v, X&& ... x) const {
	  Json& r = this->get(std::forward<T>(v)); return r.is_null() ? r : r.get(std::forward<X>(x)...);
	}
	template <typename T>
	inline void _ref(T& $) {
	  if constexpr (is_box<T>::value) {
		if(!_h)_h = new(xx::alloc()) _H(_obj_t());
		switch (_h->type) {
		case t_object: box_pack_t<T>::from_json(this, $.p);
		}
	  } else if constexpr (fc::is_vector<T>::value) {
		if(!_h)_h = new(xx::alloc()) _H(_arr_t());
		switch (_h->type) {
		case t_array:;
		}
	  }
	}
	inline void _ref(bool& $) { $ = this->as_bool(); }
	inline void _ref(float& $) { $ = static_cast<float>(this->as_double()); }
	inline void _ref(double& $) { $ = this->as_double(); }
	inline void _ref(i8& $) { $ = static_cast<i8>(this->as_int64()); }
	inline void _ref(i16& $) { $ = static_cast<i16>(this->as_int64()); }
	inline void _ref(i32& $) { $ = this->as_int(); }
	inline void _ref(i64& $) { $ = this->as_int64(); }
	inline void _ref(u8& $) { $ = static_cast<u8>(this->as_uint64()); }
	inline void _ref(u16& $) { $ = static_cast<u16>(this->as_uint64()); }
	inline void _ref(u32& $) { $ = this->as_uint(); }
	inline void _ref(u64& $) { $ = this->as_uint64(); }
	inline void _ref(std::string& $) { $ = this->as_c_str(); }
	inline void _ref(fc::Buf& $) { $ = this->as_string(); }
	inline void _ref(tm& $) {
	  const char* c = this->as_string().c_str();
	  int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
	  if (sscanf(c, "%4d-%2d-%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min, &sec) == 6) {
		$.tm_mday = day; $.tm_hour = hour; $.tm_min = min; $.tm_sec = sec;
	  }
	  $.tm_year = year - 1900; $.tm_mon = month - 1;
	}
	template <typename T>
	Json& operator=(const box<T>& v) {
	  if (v.p) {
		T* c = v.p; i8 i = -1; fc::ForEachField(v.p, [&i, c, this](auto& t){
			this->operator[](T::$[++i]) = t;
		});
	  }
	  return *this;
	}
	// set value for Json.
	//   - The last parameter is the value, other parameters are index or key.
	//   - eg.
	//     Json x;
	//     x.set("a", "b", 0, 3);  // x-> {"a": {"b": [3]}}
	template <class T>
	inline Json& set(T&& v) { return *this = Json(std::forward<T>(v)); }
	template <class A, class B, class ...X>
	inline Json& set(A&& a, B&& b, X&& ... x) {
	  Json& r = this->_set(std::forward<A>(a)); return r.set(std::forward<B>(b), std::forward<X>(x)...);
	}
	// push v to an array.
	// if the Json calling this method is not an array, it will be reset to an array.
	Json& push_back(Json&& v) {
	  if (_h && (_h->type & t_array)) {
		if (unlikely(!_h->p)) new(&_h->p) xx::Array(8);
	  } else {
		this->reset(); _h = new(xx::alloc()) _H(_arr_t()); new(&_h->p) xx::Array(8);
	  }
	  _array().push_back(v._h); v._h = 0; return *this;
	}
	Json& push_back(Json& v) { return this->push_back(std::move(v)); }
	// remove the ith element from an array
	// the last element will be moved to the ith place
	void remove(u32 i) {
	  if (this->is_array() && i < this->array_size()) { ((Json&)_array()[i]).reset(); _array().remove(i); }
	}
	void remove(int i) { this->remove((u32)i); }
	void remove(const char* key);
	// erase the ith element from an array
	void erase(u32 i) {
	  if (this->is_array() && i < this->array_size()) { ((Json&)_array()[i]).reset(); _array().erase(i); }
	}
	void erase(int i) { this->erase((u32)i); }
	void erase(const char* key);
	// it is better to use get() instead of this method.
	Json& operator[](u32 i) const { assert(this->is_array() && !_array().empty()); return (Json&)_array()[i]; }
	Json& operator[](int i) const { return this->operator[]((u32)i); }
	bool operator==(bool v) const { return this->is_bool() && _h->b == v; }
	bool operator==(double v) const { return this->is_double() && _h->d == v; }
	bool operator==(i64 v) const { return (this->is_int() || this->is_uint()) && _h->i == v; }
	bool operator==(u64 v) const { return (this->is_int() || this->is_uint()) && static_cast<u64>(_h->i) == v; }
	bool operator==(int v) const { return this->operator==((i64)v); }
	bool operator==(u32 v) const { return this->operator==((u64)v); }
	bool operator==(long v) const { return this->operator==((i64)v); }
	bool operator==(long unsigned v) const { return this->operator==((u64)v); }
	bool operator==(const char* v) const { return this->is_string() && strcmp(_h->s, v) == 0; }
	bool operator==(const fc::Buf& v) const { return this->is_string() && v == _h->s; }
	bool operator==(const std::string& v) const { return this->is_string() && v == _h->s; }
	bool operator!=(bool v) const { return !this->operator==(v); }
	bool operator!=(double v) const { return !this->operator==(v); }
	bool operator!=(i64 v) const { return !this->operator==(v); }
	bool operator!=(u64 v) const { return !this->operator==(v); }
	bool operator!=(int v) const { return !this->operator==(v); }
	bool operator!=(u32 v) const { return !this->operator==(v); }
	bool operator!=(long v) const { return !this->operator==(v); }
	bool operator!=(long unsigned v) const { return !this->operator==(v); }
	bool operator!=(const char* v) const { return !this->operator==(v); }
	bool operator!=(const fc::Buf& v) const { return !this->operator==(v); }
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
	bool empty() const { return this->size() == 0; }
	u32 array_size() const {
	  if (_h && (_h->type & t_array)) {
		return _h->p ? _array().size() : 0;
	  }
	  return 0;
	}
	u32 object_size() const {
	  if (_h && (_h->type & t_object)) {
		return _h->p ? (_array().size() >> 1) : 0;
	  }
	  return 0;
	}
	u32 string_size() const {
	  return (_h && (_h->type & t_string)) ? _h->size : 0;
	}
	// push key-value to the back of an object, key may be repeated.
	// if the Json calling this method is not an object, it will be reset to an object.
	Json& add_member(const char* key, Json&& v) {
	  if (_h && (_h->type & t_object)) {
		if (unlikely(!_h->p)) new(&_h->p) xx::Array(16);
	  } else {
		this->reset();
		_h = new(xx::alloc()) _H(_obj_t());
		new(&_h->p) xx::Array(16);
	  }
	  _array().push_back(xx::alloc_string(key, strlen(key))); // key
	  _array().push_back(v._h);
	  v._h = 0;
	  return *this;
	}
	Json& add_member(const char* key, Json& v) {
	  this->add_member(key, std::move(v)); return *this;
	}
	bool has_member(const char* key) const;
	// it is better to use get(key) instead of this method.
	Json& operator[](const char* key) const;
	class iterator {
	public:
	  typedef void* T;
	  iterator(T* p, T* e, u32 step): _p(p), _e(e), _step(step) {}
	  struct End {}; // fake end
	  static const End& end() { static End kEnd; return kEnd; }
	  bool operator!=(const End&) const { return _p != _e; }
	  bool operator==(const End&) const { return _p == _e; }
	  iterator& operator++() { _p += _step; return *this; }
	  iterator operator++(int) = delete;
	  const char* key() const { return (const char*)_p[0]; }
	  Json& value() const { return (Json&)_p[1]; }
	  Json& operator*() const { return (Json&)_p[0]; }
	private:
	  T* _p;
	  T* _e;
	  u32 _step;
	};
	// the begin iterator.
	//   - If Json is not array or object type, the return value is equal to the 
	//     end iterator.
	iterator begin() const {
	  if (_h && _h->p && (_h->type & (t_array | t_object))) {
		static_assert(t_array == 16 && t_object == 32, "");
		xx::Array& a = _array();
		return iterator(a.data(), a.data() + a.size(), _h->type >> 4);
	  }
	  return iterator(0, 0, 0);
	}
	// a fake end iterator
	const iterator::End& end() const { return iterator::end(); }
	// Stringify.
	//   - str() converts Json to minified string.
	//   - dbg() like the str(), but will truncate long string type (> 512 bytes).
	//   - dump() converts Json to human readable string with indent.
	//   - mdp: max decimal places for float point numbers.
	fc::Buf& str(fc::Buf& s, int mdp = 16)    const { return this->_json2str(s, false, mdp); }
	fc::Buf& str(fc::Buf&& s = fc::Buf(), int mdp = 16)    const { return this->_json2str(s, false, mdp); }
	fc::Buf& dbg(fc::Buf& s, int mdp = 16)    const { return this->_json2str(s, true, mdp); }
	fc::Buf& dbg(fc::Buf&& s = fc::Buf(), int mdp = 16)    const { return this->_json2str(s, true, mdp); }
	fc::Buf& dump(fc::Buf& s, int indent = 4, int mdp = 16) const { return this->_json2pretty(s, indent, indent, mdp); }
	fc::Buf& dump(int indent = 2, int mdp = 16, fc::Buf&& s = fc::Buf()) const { return this->_json2pretty(s, indent, indent, mdp); }
	fc::Buf pretty(int indent = 4, int mdp = 16) const { fc::Buf s(0x100); this->dump(s, indent, mdp); return s; }
	// Parse Json from string, inverse to stringify.
	bool parse_from(const char* s, size_t n);
	bool parse_from(const char* s) { return this->parse_from(s, strlen(s)); }
	bool parse_from(const std::string& s) { return this->parse_from(s.data(), s.size()); }
	void reset();
	void swap(Json& v) noexcept { Json::_H* h = _h; _h = v._h; v._h = h; }
	void swap(Json&& v) noexcept { v.swap(*this); }
  private:
	friend class Parser;
	void* _dup() const;
	xx::Array& _array() const { return (xx::Array&)_h->p; }
	Json& _set(u32 i);
	Json& _set(int i) { return this->_set((u32)i); }
	Json& _set(const char* key);
	fc::Buf& _json2str(fc::Buf& fs, bool debug, int& mdp) const;
	fc::Buf& _json2pretty(fc::Buf& fs, int& indent, int n, int& mdp) const;
  private:
	_H* _h;
  };
  // make an empty array
  inline Json array() { return Json(Json::_arr_t()); }
  // make an array from initializer_list
  Json array(std::initializer_list<Json> v);
  // make an empty object
  inline Json object() { return Json(Json::_obj_t()); }
  // make an object from initializer_list
  Json object(std::initializer_list<Json> v);
  inline Json parse(const char* s, size_t n) { Json r; if (r.parse_from(s, n)) return r; r.reset(); return r; }
  inline Json parse(const char* s) { return parse(s, strlen(s)); }
  inline Json parse(const std::string& s) { return parse(s.data(), s.size()); }
  inline Json parse(const fc::Buf&& b) { return parse(b.data_, b.size()); }
  inline Json parse(const fc::Buf& b) { return parse(b.data_, b.size()); }
} // json
typedef json::Json Json;
inline fc::Buf& operator<<(fc::Buf& fs, const json::Json& x) { return x.dbg(fs); }

#define FC_TO(__VA_ARGS_) c[#__VA_ARGS_].operator= (_->__VA_ARGS_);
#define FC_FROM(__VA_ARGS_) c.get(#__VA_ARGS_)._ref(_->__VA_ARGS_);
#define REG(__VA_ARGS_,...)friend json::Json;template<typename T,typename Fn>friend constexpr void fc::ForEachField(T* value, Fn&& fn);\
  static void to_json(Json& c, const __VA_ARGS_* _) { if(_){EXP(M$(FC_TO, __VA_ARGS__))} }\
  static void from_json(const Json& c, __VA_ARGS_* _) { if(_){EXP(M$(FC_FROM, __VA_ARGS__))} }\
  private: const static char* $[NUM_ARGS(__VA_ARGS__)];const static u8 _size;static const std::string _name;\
  static std::tuple<STAR_S(__VA_ARGS_,NUM_ARGS(__VA_ARGS__),__VA_ARGS__)> Tuple;

#define CLASS(__VA_ARGS_,...)const u8 __VA_ARGS_::_size = NUM_ARGS(__VA_ARGS__);const std::string __VA_ARGS_::_name=fc::toSqlCase(#__VA_ARGS_);\
  const char* __VA_ARGS_::$[NUM_ARGS(__VA_ARGS__)] = { PROTO_N(NUM_ARGS(__VA_ARGS__),__VA_ARGS__) };\
  std::tuple<STAR_S(__VA_ARGS_, NUM_ARGS(__VA_ARGS__),__VA_ARGS__)>__VA_ARGS_::Tuple=std::make_tuple(STARS(__VA_ARGS_, NUM_ARGS(__VA_ARGS__), __VA_ARGS__));

#endif
