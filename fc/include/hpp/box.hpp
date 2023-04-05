#ifndef BOX_HPP
#define BOX_HPP
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
#include <string>
#include <iostream>
#include <memory>
#include <type_traits>
#include <new>
#include <hpp/tuple.hpp>
#include <tp/c++.h>
template <typename T> class box;
template <class T> struct is_box_impl: std::false_type {};
template <class T> struct is_box_impl<box<T>>: std::true_type {};
template <class T> using is_box = is_box_impl<std::decay_t<T>>;
template <class T> struct is_box_ptr_impl: std::false_type {};
template <class T> struct is_box_ptr_impl<box<T*>>: std::true_type {};
template <class T> using is_box_ptr = is_box_ptr_impl<std::decay_t<T>>;
namespace std {
  template <class T> struct hash<box<T*>> {
    size_t operator()(const box<T*>& o) const {
      if (o.p == nullptr) return 0; return hash<std::remove_const_t<T>>()(*o);
    }
  };
  template <class T> struct hash<box<T>> {
    size_t operator()(const box<T>& o) const {
      if (!*((bool*)(&o))) return 0; return hash<std::remove_const_t<T>>()(*o);
    }
  };
}
template <typename T>
class box {
  bool b;
public:
  T p;
  box() noexcept: p(0), b(false) {}
  box(std::nullptr_t) noexcept: p(0), b(false) {}
  template<typename U, std::enable_if_t<std::is_same<T, U>::value>* = nullptr>
  box(box<U>&& _) noexcept: p(std::move(_.p)), b(std::move(_.b)) {}
  template<typename U, std::enable_if_t<std::is_same<T, U>::value>* = nullptr>
  box(box<U>& _) noexcept: p(_.p), b(_.b) {}
  box(T&& _): p(std::move(_)), b(true) {}
  box(T& _): p(_), b(true) {}
  box(T* _): p(*_), b(_ ? true : false) {}
  template<typename... X>
  box(X&&... _) noexcept: p(std::in_place, std::forward<X>(_)...), b(true) {}
  template<class I, typename... Z>
  box(std::initializer_list<I> $, Z&&... _) noexcept: p(std::in_place, $, std::forward<Z>(_)...), b(true) {}
  template<typename... U>
  box(U&... _) noexcept: p(_...), b(true) {}
  box(const box<T>& _) : p(_.p), b(true) {}
  void operator = (T* _) { if (_) p = *_, b = true; else b = false; }
  void operator=(box<T>&& _) { if(*((bool*)(&_))) p = _.p, b = true; }
  void operator=(box<T>& _) { if(*((bool*)(&_))) p = _.p, b = true; }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  void operator=(U&& _) { p = std::move(_); b = true; }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  void operator=(U& _) { p = _; b = true; }
  void swap(box& _) noexcept { std::swap(this->p, _.p); std::swap(this->b, _.b); }
  constexpr bool has_value() const noexcept { return this->b; }
  constexpr explicit operator bool() const noexcept { return this->b; }
  T& operator*()& noexcept { return this->p; }
  constexpr const T& operator*() const& noexcept { return this->p; }
  T&& operator*()&& noexcept { return std::move(this->p); }
  constexpr const T&& operator*() const&& noexcept { return std::move(this->p); }
  __CONSTEXPR T value_or(T&& _) const { return this->b ? this->p : _; }
  __CONSTEXPR T value_or(T& _) const { return this->b ? this->p : _; }
  void reset() noexcept { p = 0; b = false; }
};
template <typename T>
class box<T*> {
  bool b;
public:
  T* p;
  box() noexcept: p(NULL), b(false) {}
  box(std::nullptr_t) noexcept: p(NULL), b(false) {}
  template<typename P, std::enable_if_t<std::is_same<T, P>::value>* = nullptr>
  box(box<P*>&& _) noexcept: p(_.p), b(_.b) { _.b = false; }
  template<typename P, std::enable_if_t<std::is_same<T, P>::value>* = nullptr>
  box(box<P*>& _) noexcept: p(_.p), b(false) {}
  box(T&& _): p(new T{ std::move(_) }), b(true) {}
  box(T& _): p(std::addressof(_)), b(false) {}
  explicit box(T* _) noexcept: p(std::addressof(*_)), b(false) {}//not use
  template<typename... V>
  box(V&&... _) noexcept: p(new T{ std::move(_)... }), b(true) {}
  template<typename... Y>
  box(Y&... _) noexcept: p(new T{ _... }), b(true) {}
  ~box() { if (b) { delete p; p = nullptr; } }
  //Automatic memory management, but be careful not to release the external, eg: box<T*> xx = new T{};
  void operator = (T* _) { if (b) delete p; p = _; b = true; }
  void operator = (T& _) { if (b) *p = _; else { p = new T(_); b = true; } }
  void operator = (T&& _) { if (b) delete p; p = new T(std::move(_)); b = true; }
  void swap(box& _) noexcept { std::swap(this->p, _.p); std::swap(this->b, _.b); }
  constexpr bool has_value() const noexcept { return this->p != nullptr; }
  constexpr explicit operator bool() const noexcept { return this->p != nullptr; }
  const T* operator->() const { return this->p; }
  T* operator->() { return this->p; }
  T& operator*() & noexcept { return *p; }
  constexpr const T& operator*() const& noexcept { return *p; }
  __CONSTEXPR T value_or(T&& _) const {
    if __CONSTEXPR(!std::is_class<T>::value)* ((bool*)(this)) = false;
    return has_value() ? *this->p : _;
  }
  __CONSTEXPR T value_or(T& _) const {
    if __CONSTEXPR(!std::is_class<T>::value)* ((bool*)(this)) = false;
    return has_value() ? *this->p : _;
  }
  void reset() noexcept { if (p) { delete p; } p = nullptr; b = false; }
};
template<typename T> struct box_pack {};
template<typename T> struct box_pack<box<T*>> { using type = T; };
template<typename T> struct box_pack<box<T>> { using type = T; };
template<typename T> using box_pack_t = typename box_pack<T>::type;
//box<T*>
template <typename T>
std::ostream& operator<<(std::ostream& s, box<T*>& b) { return s << (b.p == nullptr ? "null" : *b.p); }
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T*>& b) { return s << (b.p == nullptr ? "null" : *b.p); }
template <class T, class U>
inline constexpr bool operator==(const box<T*>& l, const box<U*>& r) { return l.p == r.p; }
template <class T, class U>
inline constexpr bool operator!=(const box<T*>& l, const box<U*>& r) { return l.p != r.p; }
template <class T, class U>
inline constexpr bool operator<(const box<T*>& l, const box<U*>& r) { return r.p && (!l.p || *l < *r); }
template <class T, class U>
inline constexpr bool operator>(const box<T*>& l, const box<U*>& r) { return l.p && (!r.p || *l > *r); }
template <class T, class U>
inline constexpr bool operator<=(const box<T*>& l, const box<U*>& r) { return !l.p || (r.p && *l <= *r); }
template <class T, class U>
inline constexpr bool operator>=(const box<T*>& l, const box<U*>& r) { return !r.p || (l.p && *l >= *r); }
/// Compares an box to a `nullptr`
template <class T>
inline constexpr bool operator==(const box<T*>& l, std::nullptr_t) noexcept { return !l.p; }
template <class T>
inline constexpr bool operator==(std::nullptr_t, const box<T*>& r) noexcept { return !r.p; }
template <class T>
inline constexpr bool operator!=(const box<T*>& l, std::nullptr_t) noexcept { return l.p; }
template <class T>
inline constexpr bool operator!=(std::nullptr_t, const box<T*>& r) noexcept { return r.p; }
template <class T>
inline constexpr bool operator<(const box<T*>&, std::nullptr_t) noexcept { return false; }
template <class T>
inline constexpr bool operator<(std::nullptr_t, const box<T*>& r) noexcept { return r.p; }
template <class T>
inline constexpr bool operator<=(const box<T*>& l, std::nullptr_t) noexcept { return !l.p; }
template <class T>
inline constexpr bool operator<=(std::nullptr_t, const box<T*>&) noexcept { return true; }
template <class T>
inline constexpr bool operator>(const box<T*>& l, std::nullptr_t) noexcept { return l.p; }
template <class T>
inline constexpr bool operator>(std::nullptr_t, const box<T*>&) noexcept { return false; }
template <class T>
inline constexpr bool operator>=(const box<T*>&, std::nullptr_t) noexcept { return true; }
template <class T>
inline constexpr bool operator>=(std::nullptr_t, const box<T*>& r) noexcept { return !r.p; }
/// Compares the box with a value.
template <class T, class U>
inline constexpr bool operator==(const box<T*>& l, const U& r) { return l.p ? *l.p == r : false; }
template <class T, class U>
inline constexpr bool operator==(const U& l, const box<T*>& r) { return r.p ? l == *r.p : false; }
template <class T, class U>
inline constexpr bool operator!=(const box<T*>& l, const U& r) { return l.p ? *l.p != r : true; }
template <class T, class U>
inline constexpr bool operator!=(const U& l, const box<T*>& r) { return r.p ? l != *r.p : true; }
template <class T, class U>
inline constexpr bool operator<(const box<T*>& l, const U& r) { return l.p ? *l.p < r : true; }
template <class T, class U>
inline constexpr bool operator<(const U& l, const box<T*>& r) { return r.p ? l < *r.p : false; }
template <class T, class U>
inline constexpr bool operator<=(const box<T*>& l, const U& r) { return l.p ? *l.p <= r : true; }
template <class T, class U>
inline constexpr bool operator<=(const U& l, const box<T*>& r) { return r.p ? l <= *r.p : false; }
template <class T, class U>
inline constexpr bool operator>(const box<T*>& l, const U& r) { return l.p ? *l.p > r : false; }
template <class T, class U>
inline constexpr bool operator>(const U& l, const box<T*>& r) { return r.p ? l > *r.p : true; }
template <class T, class U>
inline constexpr bool operator>=(const box<T*>& l, const U& r) { return l.p ? *l.p >= r : false; }
template <class T, class U>
inline constexpr bool operator>=(const U& l, const box<T*>& r) { return r.p ? l >= *r.p : true; }
//box<T>
template <typename T>
std::ostream& operator<<(std::ostream& s, box<T>& b) { return s << (*((bool*)(&b)) ? "null" : b.p); }
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T>& b) { return s << (*((bool*)(&b)) ? "null" : b.p); }
template <class T, class U>
inline constexpr bool operator==(const box<T>& l, const box<U>& r) { return l.p == r.p; }
template <class T, class U>
inline constexpr bool operator!=(const box<T>& l, const box<U>& r) { return l.p != r.p; }
template <class T, class U>
inline constexpr bool operator<(const box<T>& l, const box<U>& r) { return *((bool*)(&r)) && (!*((bool*)(&l)) || *l < *r); }
template <class T, class U>
inline constexpr bool operator>(const box<T>& l, const box<U>& r) { return *((bool*)(&l)) && (!*((bool*)(&r)) || *l > *r); }
template <class T, class U>
inline constexpr bool operator<=(const box<T>& l, const box<U>& r) { return !*((bool*)(&l)) || (*((bool*)(&r)) && *l <= *r); }
template <class T, class U>
inline constexpr bool operator>=(const box<T>& l, const box<U>& r) { return !*((bool*)(&r)) || (*((bool*)(&l)) && *l >= *r); }
/// Compares an box to a `nullptr`
template <class T>
inline constexpr bool operator==(const box<T>& l, std::nullptr_t) noexcept { return !*((bool*)(&l)); }
template <class T>
inline constexpr bool operator==(std::nullptr_t, const box<T>& r) noexcept { return !*((bool*)(&r)); }
template <class T>
inline constexpr bool operator!=(const box<T>& l, std::nullptr_t) noexcept { return *((bool*)(&l)); }
template <class T>
inline constexpr bool operator!=(std::nullptr_t, const box<T>& r) noexcept { return *((bool*)(&r)); }
template <class T>
inline constexpr bool operator<(const box<T>&, std::nullptr_t) noexcept { return false; }
template <class T>
inline constexpr bool operator<(std::nullptr_t, const box<T>& r) noexcept { return *((bool*)(&r)); }
template <class T>
inline constexpr bool operator<=(const box<T>& l, std::nullptr_t) noexcept { return !*((bool*)(&l)); }
template <class T>
inline constexpr bool operator<=(std::nullptr_t, const box<T>&) noexcept { return true; }
template <class T>
inline constexpr bool operator>(const box<T>& l, std::nullptr_t) noexcept { return *((bool*)(&l)); }
template <class T>
inline constexpr bool operator>(std::nullptr_t, const box<T>&) noexcept { return false; }
template <class T>
inline constexpr bool operator>=(const box<T>&, std::nullptr_t) noexcept { return true; }
template <class T>
inline constexpr bool operator>=(std::nullptr_t, const box<T>& r) noexcept { return !*((bool*)(&r)); }
/// Compares the box with a value.
template <class T, class U>
inline constexpr bool operator==(const box<T>& l, const U& r) { return *((bool*)(&l)) ? l.p == r : false; }
template <class T, class U>
inline constexpr bool operator==(const U& l, const box<T>& r) { return *((bool*)(&r)) ? l == r.p : false; }
template <class T, class U>
inline constexpr bool operator!=(const box<T>& l, const U& r) { return *((bool*)(&l)) ? l.p != r : true; }
template <class T, class U>
inline constexpr bool operator!=(const U& l, const box<T>& r) { return *((bool*)(&r)) ? l != r.p : true; }
template <class T, class U>
inline constexpr bool operator<(const box<T>& l, const U& r) { return *((bool*)(&l)) ? l.p < r : true; }
template <class T, class U>
inline constexpr bool operator<(const U& l, const box<T>& r) { return *((bool*)(&r)) ? l < r.p : false; }
template <class T, class U>
inline constexpr bool operator<=(const box<T>& l, const U& r) { return *((bool*)(&l)) ? l.p <= r : true; }
template <class T, class U>
inline constexpr bool operator<=(const U& l, const box<T>& r) { return *((bool*)(&r)) ? l <= r.p : false; }
template <class T, class U>
inline constexpr bool operator>(const box<T>& l, const U& r) { return *((bool*)(&l)) ? l.p > r : false; }
template <class T, class U>
inline constexpr bool operator>(const U& l, const box<T>& r) { return *((bool*)(&r)) ? l > r.p : true; }
template <class T, class U>
inline constexpr bool operator>=(const box<T>& l, const U& r) { return *((bool*)(&l)) ? l.p >= r : false; }
template <class T, class U>
inline constexpr bool operator>=(const U& l, const box<T>& r) { return *((bool*)(&r)) ? l >= r.p : true; }
#include <vector>
template<typename T>
using vec = std::vector<T>;
#endif // BOX_HPP
