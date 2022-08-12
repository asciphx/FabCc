#ifndef JSON_H
#define JSON_H
/* MIT License

Copyright (c) 2019-2022 Alvin Yih idealvin@qq.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef _MSC_VER
#pragma warning (disable:4200)
#ifndef __thread
#define __thread __declspec(thread)
#endif
#endif
#include "str.hh"
#include "buffer.hh"
#include <cassert>
#include <initializer_list>
// from https://github.com/idealvin/coost/blob/master/include/co/json.h
namespace json {
  namespace xx {
	class Array {
	public:
	  typedef void* T;
	  struct _H {
		u32 cap;
		u32 size;
		T p[];
	  };
	  static const size_t N = sizeof(T);
	  static const u32 R = sizeof(_H) / N;
	  explicit Array(u32 cap) {
		_h = (_H*) ::malloc(N * (R + cap));
		_h->cap = cap;
		_h->size = 0;
	  }
	  Array(): Array(1024 - R) {}
	  ~Array() {
		::free(_h);
	  }
	  T* data() const { return _h->p; }
	  u32 size() const { return _h->size; }
	  bool empty() const { return this->size() == 0; }
	  void resize(u32 n) { _h->size = n; }
	  T& back() const { return _h->p[this->size() - 1]; }
	  T& operator[](u32 i) const { return _h->p[i]; }
	  void push_back(T v) {
		if (_h->size == _h->cap) {
		  const size_t n = N * _h->cap;
		  const size_t o = sizeof(_H) + n;
		  _h = (_H*) ::realloc(_h, o + n); assert(_h);
		  _h->cap <<= 1;
		}
		_h->p[_h->size++] = v;
	  }
	  void remove(u32 i) {
		if (i != --_h->size) _h->p[i] = _h->p[_h->size];
	  }

	  void remove_pair(u32 i) {
		if (i != (_h->size -= 2)) {
		  _h->p[i] = _h->p[_h->size];
		  _h->p[i + 1] = _h->p[_h->size + 1];
		}
	  }
	  T pop_back() {
		return _h->p[--_h->size];
	  }
	  void reset() {
		_h->size = 0;
		if (_h->cap > 8192) {
		  ::free(_h);
		  new(this) Array();
		}
	  }
	private:
	  _H* _h;
	};
	void* alloc();
	char* alloc_string(const void* p, size_t n);
  } // xx
  class Json {
  public:
	enum {
	  t_bool = 1,
	  t_int = 2,
	  t_double = 4,
	  t_string = 8,
	  t_array = 16,
	  t_object = 32,
	};
	struct _obj_t {};
	struct _arr_t {};
	struct _H {
	  _H(bool v) noexcept: type(t_bool), b(v) {}
	  _H(i64 v) noexcept: type(t_int), i(v) {}
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
		i64 i;  // for int
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
	  if (&v != this) {
		if (_h) this->reset();
		_h = v._h;
		v._h = 0;
	  }
	  return *this;
	}
	// after this operation, v will be moved and becomes null
	Json& operator=(Json& v) {
	  return this->operator=(std::move(v));
	}
	// make a duplicate 
	Json dup() const {
	  Json r;
	  r._h = (_H*)this->_dup();
	  return r;
	}
	Json(bool v): _h(new(xx::alloc()) _H(v)) {}
	Json(double v): _h(new(xx::alloc()) _H(v)) {}
	Json(i64 v): _h(new(xx::alloc()) _H(v)) {}
	Json(i32 v): Json((i64)v) {}
	Json(u32 v): Json((i64)v) {}
	Json(u64 v): Json((i64)v) {}
	// for string type
	Json(const void* p, size_t n): _h(new(xx::alloc()) _H(p, n)) {}
	Json(const char* s): Json(s, strlen(s)) {}
	Json(const std::string& s): Json(s.data(), s.size()) {}
	Json(_obj_t): _h(new(xx::alloc()) _H(_obj_t())) {}
	Json(_arr_t): _h(new(xx::alloc()) _H(_arr_t())) {}
	// make Json from initializer_list
	Json(std::initializer_list<Json> v);
	bool is_null() const { return _h == 0; }
	bool is_bool() const { return _h && (_h->type & t_bool); }
	bool is_int() const { return _h && (_h->type & t_int); }
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
		case t_string: return fc::to_bool(_h->s);
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
		case t_string: return fc::to_int64(_h->s);
		case t_double: return (i64)_h->d;
		case t_bool:   return _h->b ? 1 : 0;
		}
	  }
	  return 0;
	}
	i32 as_int32() const { return (i32)this->as_int64(); }
	int as_int() const { return (int)this->as_int64(); }
	// try to get a double value
	//   - string or integer type, convert to double
	//   - bool type, true -> 1, false -> 0
	//   - other non-double types, -> 0
	double as_double() const {
	  if (_h) {
		switch (_h->type) {
		case t_double: return _h->d;
		case t_int:    return (double)_h->i;
		case t_string: return fc::to_double(_h->s);
		case t_bool:   return _h->b ? 1 : 0;
		}
	  }
	  return 0;
	}
	// returns a c-style string, null-terminated.
	// for non-string types, returns an empty string.
	const char* as_c_str() const {
	  return this->is_string() ? _h->s : "";
	}
	// returns a std::string.
	// for non-string types, it is equal to Json::str().
	std::string as_string() const {
	  return this->is_string() ? std::string(_h->s, _h->size) : this->str(fc::Buffer()).b2s();
	}
	// get Json by index or key.
	//   - It is a read-only operation.
	//   - If the index is not in a valid range or the key does not exist, 
	//     the return value is a reference to a null object.
	Json& get() const { return *(Json*)this; }
	Json& get(u32 i) const;
	Json& get(int i) const { return this->get((u32)i); }
	Json& get(const char* key) const;
	template <class T, class ...X>
	inline Json& get(T&& v, X&& ... x) const {
	  auto& r = this->get(std::forward<T>(v));
	  return r.is_null() ? r : r.get(std::forward<X>(x)...);
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
	  auto& r = this->_set(std::forward<A>(a));
	  return r.set(std::forward<B>(b), std::forward<X>(x)...);
	}
	// push v to an array.
	// if the Json calling this method is not an array, it will be reset to an array.
	Json& push_back(Json&& v) {
	  if (_h && (_h->type & t_array)) {
		if (unlikely(!_h->p)) new(&_h->p) xx::Array(8);
	  } else {
		this->reset();
		_h = new(xx::alloc()) _H(_arr_t());
		new(&_h->p) xx::Array(8);
	  }
	  _array().push_back(v._h);
	  v._h = 0;
	  return *this;
	}
	Json& push_back(Json& v) {
	  return this->push_back(std::move(v));
	}
	// remove the ith element from an array
	void remove(u32 i) {
	  if (this->is_array() && i < this->array_size()) {
		((Json&)_array()[i]).reset();
		_array().remove(i);
	  }
	}
	void remove(int i) { this->remove((u32)i); }
	void remove(const char* key);
	// it is better to use get() instead of this method.
	Json& operator[](u32 i) const {
	  assert(this->is_array() && !_array().empty());
	  return (Json&)_array()[i];
	}
	Json& operator[](int i) const {
	  return this->operator[]((u32)i);
	}
	bool operator==(bool v) const { return this->is_bool() && _h->b == v; }
	bool operator==(double v) const { return this->is_double() && _h->d == v; }
	bool operator==(i64 v) const { return this->is_int() && _h->i == v; }
	bool operator==(int v) const { return this->operator==((i64)v); }
	bool operator==(u32 v) const { return this->operator==((i64)v); }
	bool operator==(u64 v) const { return this->operator==((i64)v); }
	bool operator==(const char* v) const { return this->is_string() && strcmp(_h->s, v) == 0; }
	bool operator==(const std::string& v) const { return this->is_string() && v == _h->s; }
	bool operator!=(bool v) const { return !this->operator==(v); }
	bool operator!=(double v) const { return !this->operator==(v); }
	bool operator!=(i64 v) const { return !this->operator==(v); }
	bool operator!=(int v) const { return !this->operator==(v); }
	bool operator!=(u32 v) const { return !this->operator==(v); }
	bool operator!=(u64 v) const { return !this->operator==(v); }
	bool operator!=(const char* v) const { return !this->operator==(v); }
	bool operator!=(const std::string& v) const { return !this->operator==(v); }
	// for array and object, return number of the elements.
	// for string, return the length.
	// for other types, return 0.
	u32 size() const {
	  if (_h) {
		switch (_h->type) {
		case t_array:
		  return _h->p ? _array().size() : 0;
		case t_object:
		  return _h->p ? (_array().size() >> 1) : 0;
		case t_string:
		  return _h->size;
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
	  this->add_member(key, std::move(v));
	  return *this;
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
		auto& a = _array();
		return iterator(a.data(), a.data() + a.size(), _h->type >> 4);
	  }
	  return iterator(0, 0, 0);
	}
	// a fake end iterator
	const iterator::End& end() const { return iterator::end(); }
	// Stringify.
	//   - str() converts Json to minified string.
	//   - dbg() like the str(), but will truncate long string type (> 512 bytes).
	//   - pretty() converts Json to human readable string.
	//   - mdp: max decimal places for float point numbers.
	fc::Buffer& str(int mdp = 16)    const { return this->_json2str(fc::Buffer(), false, mdp); }
	fc::Buffer& dump(int mdp = 16) const { return this->_json2pretty(fc::Buffer(), 2, 2, mdp); }
	fc::Buffer& str(fc::Buffer& s, int mdp = 16)    const { return this->_json2str(s, false, mdp); }
	fc::Buffer& dbg(fc::Buffer& s, int mdp = 16)    const { return this->_json2str(s, true, mdp); }
	fc::Buffer& pretty(fc::Buffer& s, int mdp = 16) const { return this->_json2pretty(s, 4, 4, mdp); }
	// Parse Json from string, inverse to stringify.
	bool parse_from(const char* s, size_t n);
	bool parse_from(const char* s) { return this->parse_from(s, strlen(s)); }
	bool parse_from(const std::string& s) { return this->parse_from(s.data(), s.size()); }
	void reset();
	void swap(Json& v) noexcept { auto h = _h; _h = v._h; v._h = h; }
	void swap(Json&& v) noexcept { v.swap(*this); }
  private:
	friend class Parser;
	void* _dup() const;
    xx::Array& _array() const { return (xx::Array&)_h->p; }
	Json& _set(u32 i);
	Json& _set(int i) { return this->_set((u32)i); }
	Json& _set(const char* key);
	fc::Buffer& _json2str(fc::Buffer& fs, bool debug, int mdp) const;
	fc::Buffer& _json2pretty(fc::Buffer& fs, int indent, int n, int mdp) const;
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
  inline Json parse(const char* s, size_t n) {
	Json r;
	if (r.parse_from(s, n)) return r;
	r.reset();
	return r;
  }
  inline Json parse(const char* s) { return parse(s, strlen(s)); }
  inline Json parse(const std::string& s) { return parse(s.data(), s.size()); }
} // json
typedef json::Json Json;
inline fc::Buffer& operator<<(fc::Buffer& fs, const json::Json& x) { return x.dbg(fs); }
#endif
