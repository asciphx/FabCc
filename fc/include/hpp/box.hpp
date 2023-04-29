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
#include <stdexcept>
#include <memory>
#include <type_traits>
#include <new>
#include <hpp/tuple.hpp>
#include <tp/c++.h>
template <typename T> class box;
template <class T> struct is_box_impl: std::false_type {};
template <class T> struct is_box_impl<box<T>>: std::true_type {};
template <class T> using is_box = is_box_impl<std::decay_t<T>>;
namespace std {
  template <class T> struct hash<box<T>> {
    size_t operator()(const box<T>& o) const {
      if (o.p == nullptr) return 0; return hash<std::remove_const_t<T>>()(*o);
    }
  };
}
template <class T, class... K>
inline constexpr box<T> make_box(K &&... k) { return box<T>(std::forward<K>(k)...); }
template <typename T>
class box {
  bool b;
public:
  T* p;
  box() noexcept: p(NULL), b(false) {}
  box(std::nullptr_t) noexcept: p(NULL), b(false) {}
  box(box<T>&& _) noexcept: p(_.p), b(_.b) { _.b = false; }
  box(box<T>& _) noexcept: p(_.p), b(false) {}
  box(T&& _) noexcept: p(new T{ std::move(_) }), b(true) {}
  box(T& _) noexcept: p(std::addressof(_)), b(false) {}
  box(const box<T>& _) noexcept: p(_.p), b(_.b) { const_cast<box<T>&>(_).b = false; }
  explicit box(T* _) noexcept: p(std::addressof(*_)), b(false) {}//not use
  template<typename... X>
  box(X&&... _) noexcept: p(new T{ std::forward<X>(_)... }), b(true) {}
  ~box() noexcept { if (this->b) { delete this->p; this->p = nullptr; } }
  //Automatic memory management, but be careful not to release the external, eg: box<T> xx = new T{};
  FORCE_INLINE void operator = (T* _) noexcept { if (this->b) delete this->p; this->p = _; this->b = true; }
  FORCE_INLINE void operator = (T& _) noexcept { if (this->b) *this->p = _; else { this->p = new T(_); this->b = true; } }
  FORCE_INLINE void operator = (T&& _) noexcept { if (this->b) delete this->p; this->p = new T(std::move(_)); this->b = true; }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  FORCE_INLINE void operator=(U&& _) noexcept {
    if (this->p) *this->p = std::move(_); else { this->p = new T(std::move(_)); this->b = true; }
  }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = nullptr>
  FORCE_INLINE void operator=(U& _) noexcept {
    if (this->p) *this->p = _; else { this->p = new T(_); this->b = true; }
  }
  FORCE_INLINE void swap(box& _) noexcept { std::swap(this->p, _.p); std::swap(this->b, _.b); }
  constexpr bool has_value() const noexcept { return this->p != nullptr; }
  constexpr explicit operator bool() const noexcept { return this->p != nullptr; }
  const T* operator->() const { if (!p)throw std::range_error(std::string(typeid(T).name()).append("'s ptr is null!", 15)); return p; }
  T* operator->() { if (!this->p)throw std::range_error(std::string(typeid(T).name()).append("'s ptr is null!", 15)); return this->p; }
  FORCE_INLINE T& operator*() & noexcept { return *this->p; }
  constexpr const T& operator*() const& noexcept { return *this->p; }
  __CONSTEXPR T value_or(T&& _) const noexcept { return this->p != nullptr ? *this->p : _; }
  __CONSTEXPR T value_or(T& _) const noexcept { return this->p != nullptr ? *this->p : _; }
  FORCE_INLINE void reset() noexcept { if (this->p) { this->p->~T(); ::free(static_cast<void*>(this->p)); } this->b = false; }
};
template<typename T> struct box_pack {};
template<typename T> struct box_pack<box<T>> { using type = T; };
template<typename T> using box_pack_t = typename box_pack<T>::type;
//box<T>
template <typename T>
std::ostream& operator<<(std::ostream& s, box<T>& b) { if (b.p == nullptr) return s << "null"; return s << *b.p; }
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T>& b) { if (b.p == nullptr) return s << "null"; return s << *b.p; }
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
#include <vector>
template<typename T>
using vec = std::vector<T>;
#endif // BOX_HPP
