#ifndef BOX_H
#define BOX_H
#include <string>
#include <iostream>
#include <memory>
#include <type_traits>
#include <new>
#include <tp/c++.h>
template <typename T> class box;
template <class T> struct is_box_impl: std::false_type {};
template <class T> struct is_box_impl<box<T>>: std::true_type {};
template <class T> using is_box = is_box_impl<std::decay_t<T>>;
template <typename T>
class box {
  bool b;
public:
  T* p;
  box() noexcept: p(NULL), b(false) {}
  box(std::nullptr_t) noexcept: p(NULL), b(false) {}
  template<typename U>
  box(box<U>&& x) noexcept: p(x.p), b(true) { x.p = 0; x.hide(); }
  template<typename U>
  box(box<U>& x) noexcept: p(x.p), b(false) {}
#ifdef _WIN32
  box(T&& _) : p(new T(std::move(_))), b(true) {}
#else
  box(T&& _) : p(new T(std::move(_))), b(false) {}
#endif // _WIN32
  box(T& _): p(&_), b(false) {}
  explicit box(T* t) noexcept: p(t), b(false) {}
  template<typename... U>
  box(U&&... t) noexcept: p(new T{ std::move(t)... }), b(true) {}
  template<typename... U>
  box(U&... t) noexcept: p(new T{ t... }), b(true) {}
  ~box() { if (b) { delete p; } }
  void operator = (T* s) { if (b)delete p; p = s; b = false; }
  void operator = (T& s) { if (b)delete p; p = &s; b = false; }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  void operator=(U&& u) {
	if (b) *p = std::move(u); else { p = new T(std::move(u)); b = true; }
  }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  void operator=(U& u) {
	if (b) *p = u, b = false; else { p = new T(u); b = true; }
  }
  T** operator&() { return &p; }
  __CONSTEXPR const T* operator->() const { return this->p; }
  T* operator->() { return this->p; }
  T& operator() () & noexcept { return *p; }
  __CONSTEXPR const T& operator() () const& noexcept { return *p; }
  T& operator*() & noexcept { return *p; }
  __CONSTEXPR const T& operator*() const& noexcept { return *p; }
  __CONSTEXPR T value_or(T&& u) const {
	if __CONSTEXPR(!std::is_class<T>::value)
#ifdef _WIN32
	  * ((bool*)(this)) = false;
#else
	  * ((bool*)(this)) = true;
#endif // _WIN32
	return this->p ? *std::addressof(*this->operator->()) : u;
  }
  __CONSTEXPR T value_or(T& u) const {
	if __CONSTEXPR(!std::is_class<T>::value)
#ifdef _WIN32
	  * ((bool*)(this)) = false;
#else
	  * ((bool*)(this)) = true;
#endif // _WIN32
	return this->p ? **&this->operator->() : u;
  }
  //if in the any container, eg:std::map{ { box<T>, U } } , and not used value_or
  void clear() const noexcept {
	if __CONSTEXPR(!std::is_class<T>::value)
#ifdef _WIN32
	  * ((bool*)(this)) = false;
#else
	  * ((bool*)(this)) = true;
#endif // _WIN32
}
  void reset() noexcept { if (b) { b = false; delete p; } }
private:
  void hide() { this->b = false; }
};
namespace std {
  template <class T> struct hash<box<T>> {
	size_t operator()(const box<T>& o) const {
	  if (o.p == nullptr) return 0; return hash<std::remove_const_t<T>>()(*o);
	}
  };
}
template <class T, class... K>
inline constexpr box<T> make_box(K &&... k) { return box<T>(std::in_place, std::forward<K>(k)...); }
template <class T, class U, class... K>
inline constexpr box<T> make_box(std::initializer_list<U> i, K &&... k) {
  return box<T>(std::in_place, i, std::forward<K>(k)...);
}
template <typename T>
std::ostream& operator<<(std::ostream& s, box<T>& c) { return s << (c.p == nullptr ? "null" : *c.p); }
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T>& c) { return s << (c.p == nullptr ? "null" : *c.p); }
template <class T, class U>
inline constexpr bool operator==(const box<T>& l, const box<U>& r) { return l.p == r.p; }
template <class T, class U>
inline constexpr bool operator!=(const box<T>& l, const box<U>& r) { return l.p != r.p; }
template <class T, class U>
inline constexpr bool operator<(const box<T>& l, const box<U>& r) { return r.p && (!l.p || *l < *r); }
template <class T, class U>
inline constexpr bool operator>(const box<T>& l, const box<U>& r) { return l.p && (!r.p || *l > *r); }
template <class T, class U>
inline constexpr bool operator<=(const box<T>& l, const box<U>& r) { return !l.p || (r.p && *l <= *r); }
template <class T, class U>
inline constexpr bool operator>=(const box<T>& l, const box<U>& r) { return !r.p || (l.p && *l >= *r); }
/// Compares an box to a `nullptr`
template <class T>
inline constexpr bool operator==(const box<T>& l, std::nullptr_t) noexcept { return !l.p; }
template <class T>
inline constexpr bool operator==(std::nullptr_t, const box<T>& r) noexcept { return !r.p; }
template <class T>
inline constexpr bool operator!=(const box<T>& l, std::nullptr_t) noexcept { return l.p; }
template <class T>
inline constexpr bool operator!=(std::nullptr_t, const box<T>& r) noexcept { return r.p; }
template <class T>
inline constexpr bool operator<(const box<T>&, std::nullptr_t) noexcept { return false; }
template <class T>
inline constexpr bool operator<(std::nullptr_t, const box<T>& r) noexcept { return r.p; }
template <class T>
inline constexpr bool operator<=(const box<T>& l, std::nullptr_t) noexcept { return !l.p; }
template <class T>
inline constexpr bool operator<=(std::nullptr_t, const box<T>&) noexcept { return true; }
template <class T>
inline constexpr bool operator>(const box<T>& l, std::nullptr_t) noexcept { return l.p; }
template <class T>
inline constexpr bool operator>(std::nullptr_t, const box<T>&) noexcept { return false; }
template <class T>
inline constexpr bool operator>=(const box<T>&, std::nullptr_t) noexcept { return true; }
template <class T>
inline constexpr bool operator>=(std::nullptr_t, const box<T>& r) noexcept { return !r.p; }
/// Compares the box with a value.
template <class T, class U>
inline constexpr bool operator==(const box<T>& l, const U& r) { return l.p ? *l.p == r : false; }
template <class T, class U>
inline constexpr bool operator==(const U& l, const box<T>& r) { return r.p ? l == *r.p : false; }
template <class T, class U>
inline constexpr bool operator!=(const box<T>& l, const U& r) { return l.p ? *l.p != r : true; }
template <class T, class U>
inline constexpr bool operator!=(const U& l, const box<T>& r) { return r.p ? l != *r.p : true; }
template <class T, class U>
inline constexpr bool operator<(const box<T>& l, const U& r) { return l.p ? *l.p < r : true; }
template <class T, class U>
inline constexpr bool operator<(const U& l, const box<T>& r) { return r.p ? l < *r.p : false; }
template <class T, class U>
inline constexpr bool operator<=(const box<T>& l, const U& r) { return l.p ? *l.p <= r : true; }
template <class T, class U>
inline constexpr bool operator<=(const U& l, const box<T>& r) { return r.p ? l <= *r.p : false; }
template <class T, class U>
inline constexpr bool operator>(const box<T>& l, const U& r) { return l.p ? *l.p > r : false; }
template <class T, class U>
inline constexpr bool operator>(const U& l, const box<T>& r) { return r.p ? l > *r.p : true; }
template <class T, class U>
inline constexpr bool operator>=(const box<T>& l, const U& r) { return l.p ? *l.p >= r : false; }
template <class T, class U>
inline constexpr bool operator>=(const U& l, const box<T>& r) { return r.p ? l >= *r.p : true; }
template<typename T> struct box_pack {};
template<typename T> struct box_pack<box<T>> { using type = T; };
template<typename T> using box_pack_t = typename box_pack<T>::type;
#include <vector>
template<typename T>
using vec = std::vector<T>;
#endif // BOX_H
