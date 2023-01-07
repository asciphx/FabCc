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
#ifdef _WIN32
  box(box<U>&& _) noexcept: p(_.p), b(_.b) { _.p = 0; _.b = false; }
#else
  box(box<U>&& _) noexcept: p(NULL), b(false) {
	if (_.p != nullptr) { p = new T{ *_.p }; delete _.p; _.p = nullptr; _.b = false; b = true; }
  }
#endif // _WIN32
  template<typename U>
  box(box<U>& _) noexcept: p(_.p), b(false) {}
#ifdef _WIN32
  box(T&& _) : p(new T{ std::move(_) }), b(true) {}
#else
  box(T&& _) : p(new T{ std::move(_) }), b(false) {}
#endif // _WIN32
  box(T& _) : p(&_), b(false) {}
  explicit box(T* _) noexcept: p(_), b(false) {}
  template<typename... U>
  box(U&&... _) noexcept: p(new T{ std::move(_)... }), b(true) {}
  template<typename... U>
  box(U&... _) noexcept: p(new T{ _... }), b(true) {}
  ~box() { if (b) { delete p; } p = nullptr; }
  //Automatic memory management, eg: box<T> xx = new T{};
  void operator = (T* s) { if (b)delete p; p = s; }
  void operator = (T& s) { if (b)delete p; p = &s; b = false; }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  void operator=(U&& _) {
	if (b) delete p; p = new T(std::move(_)); b = true;
  }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  void operator=(U& _) {
	if (b) *p = _, b = false; else { p = new T(_); b = true; }
  }
  void swap(box& _) noexcept { std::swap(this->p, _.p); std::swap(this->b, _.b); }
  constexpr bool has_value() const noexcept { return this->p != nullptr; }
  constexpr explicit operator bool() const noexcept { return this->p != nullptr; }
  constexpr const T* operator->() const { return this->p; }
  T* operator->() { return this->p; }
  T& operator() () & noexcept { return *p; }
  constexpr const T& operator() () const& noexcept { return *p; }
  T& operator*() & noexcept { return *p; }
  constexpr const T& operator*() const& noexcept { return *p; }
  __CONSTEXPR T value_or(T&& _) const {
	if __CONSTEXPR(!std::is_class<T>::value)
#ifdef _WIN32
	  * ((bool*)(this)) = false;
#else
	  * ((bool*)(this)) = true;
#endif // _WIN32
	return this->p ? *this->operator->() : _;
  }
  __CONSTEXPR T value_or(T& _) const {
	if __CONSTEXPR(!std::is_class<T>::value)
#ifdef _WIN32
	  * ((bool*)(this)) = false;
#else
	  * ((bool*)(this)) = true;
#endif // _WIN32
	return this->p ? *this->operator->() : _;
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
  void reset() noexcept { if (b) { b = false; delete p; } p = nullptr; }
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
std::ostream& operator<<(std::ostream& s, box<T>& b) { return s << (b.p == nullptr ? "null" : *b.p); }
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T>& b) { return s << (b.p == nullptr ? "null" : *b.p); }
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
