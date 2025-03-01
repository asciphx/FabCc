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
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <stack>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <new>
#ifdef _MSVC_LANG
#define _FORCE_INLINE __forceinline
#else
#define _FORCE_INLINE inline __attribute__((always_inline))
#endif
template <typename T> class box;
namespace std {
  template <class T> struct is_box_impl: std::false_type {}; template <class T> struct is_box_impl<box<T>>: std::true_type {};
  template<typename T> static auto is_ojbk(int) -> std::integral_constant<bool, std::is_same<typename T::_$_, in_place_t>::value> {};
  template<typename...> static auto is_ojbk(...) -> std::false_type; template<typename T> struct is_reg: decltype(is_ojbk<T>(0)){};
  template <class T> struct hash<box<T>> { size_t operator()(const box<T>& o) const { if (o) return hash<std::remove_const_t<T>>()(*o); return 0; } };
  template<typename T> struct box_pack {}; template<typename T> struct box_pack<box<T>> { using type = T; };
  template<typename T> using box_pack_t = typename box_pack<T>::type; template <class T> struct is_arr: std::false_type {};
  template <class T> struct is_arr<std::list<T>>: std::true_type {}; template <class T> struct is_arr<std::forward_list<T>>: std::true_type {};
  template <class T> struct is_arr<std::deque<T>>: std::true_type {}; template <class T> struct is_arr<std::vector<T>>: std::true_type {};
  template<typename C> struct arr_pack {}; template<typename C> struct set_pack {}; template <class T> struct is_set: std::false_type {};
  template <class T> struct is_set<std::set<T>>: std::true_type {}; template <class T> struct is_set<std::unordered_multiset<T>>: std::true_type {};
  template <class T> struct is_set<std::multiset<T>>: std::true_type {}; template <class T> struct is_set<std::unordered_set<T>>: std::true_type {};
  template<class C> struct is_mmp: std::false_type {}; template<class T, typename V> struct is_mmp<std::map<T, V>>: std::true_type {};
  template<class T, typename V> struct is_mmp<std::multimap<T, V>>: std::true_type {}; template <class T> using is_box = is_box_impl<std::decay_t<T>>;
  template<class T, typename V> struct is_mmp<std::unordered_map<T, V>>: std::true_type {};
  template<class T, typename V> struct is_mmp<std::unordered_multimap<T, V>>: std::true_type {};
  template<template<typename, typename> class C, typename A, typename B> struct arr_pack<C<A, B>> { using type = A; };
  template<template<typename, typename, typename> class D, typename A, typename B, typename C> struct set_pack<D<A, B, C>> { using type = A; };
  template<typename T> using arr_pack_t = typename arr_pack<T>::type; template<typename T> using set_pack_t = typename set_pack<T>::type;
}
template<typename T> using vec = std::vector<T>; template<typename T> using lis = std::forward_list<T>; template<typename T> using deq = std::deque<T>;
template <class T, class... K> inline constexpr box<T> make_box(K &&... k) { return box<T>(std::forward<K>(k)...); }
template <class T> inline constexpr box<T> make_box(T&& _) { return box<T>(std::forward<T>(_)); }
#define null nullptr
// Dark magic, automatic memory management, but be careful not to release the external
template <typename T>
class box {
  bool b; T* p;
public:
  box() noexcept: p(NULL), b(false) {}
  box(std::nullptr_t) noexcept: p(NULL), b(false) {}
  explicit box(box<T>&& _) noexcept: p(_.p), b(true) { _.p = nullptr; }
  explicit box(box<T>& _) noexcept: p(_.p), b(_.b) { _.b = false; }
  explicit box(T&& _) noexcept: p(new T{ std::move(_) }), b(true) {}
  explicit box(T& _) noexcept: p(std::addressof(_)), b(false) {}
  explicit box(const box<T>& _) noexcept: p(_.p), b(_.b) { const_cast<box<T>&>(_).b = false; }
  explicit box(T* _) noexcept: p(_), b(true) {}
  template<typename... X> box(X&&... _) noexcept: p(new T{ std::forward<X>(_)... }), b(true) {}
  ~box() noexcept { if (this->b) { delete this->p; this->p = null; } }
  _FORCE_INLINE void operator = (T* _) noexcept { if (this->b) delete this->p; this->p = _; this->b = _ ? true : false; }
  _FORCE_INLINE void operator = (box<T>& _) noexcept {
    if (this->b) { if (_.p)*this->p = *_.p; else { delete this->p; this->p = null; this->b = false; } } else if (_.p) { this->p = new T(*_.p); this->b = true; }
  }
  _FORCE_INLINE void operator = (box<T>&& _) noexcept { if (this->b) delete this->p; this->p = _.p; this->b = _.b; _.b = false; }
  _FORCE_INLINE void operator = (T& _) noexcept { if (this->b) *this->p = _; else { this->p = new T(_); this->b = true; } }
  _FORCE_INLINE void operator = (T&& _) noexcept {
    if (this->p) *this->p = std::move(_); else { if (!this->b) this->b = true; else delete this->p; this->p = new T(std::move(_)); }
  }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = null>
  _FORCE_INLINE void operator=(U&& _) noexcept {
    if (this->p) *this->p = std::move(_); else { this->p = new T(std::move(_)); this->b = true; }
  }
  template <class U = T, std::enable_if_t<!std::is_same<box<T>, std::decay_t<U>>::value>* = null>
  _FORCE_INLINE void operator=(U& _) noexcept {
    if (this->p) *this->p = _; else { this->p = new T(_); this->b = true; }
  }
  _FORCE_INLINE void swap(box& _) noexcept { std::swap(this->p, _.p); std::swap(this->b, _.b); }
  _FORCE_INLINE explicit operator bool() const noexcept { return this->p != null; }
  _FORCE_INLINE bool operator!() const noexcept { return this->p == null; }
  const T* operator->() const { if (p)return p; throw std::range_error(std::string(typeid(T).name()).append(" is null!", 9)); }
  T* operator->() { if (this->p)return this->p; throw std::range_error(std::string(typeid(T).name()).append(" is null!", 9)); }
  _FORCE_INLINE T& operator*() & { if (!this->p) throw std::range_error(std::string(typeid(T).name()).append(" is null!", 9)); return *this->p; }
  _FORCE_INLINE const T& operator*() const& { if (!p) throw std::range_error(std::string(typeid(T).name()).append(" is null!", 9)); return *p; }
  T value_or(T&& _) const noexcept { return this->p != null ? *this->p : _; }
  T value_or(T& _) const noexcept { return this->p != null ? *this->p : _; }
  _FORCE_INLINE void reset() noexcept { if (this->p) { delete this->p; } this->b = false; }
};
template<typename T, std::enable_if_t<!std::is_reg<T>::value>* = null>
std::string& operator<<(std::string& b, const box<T>& v) { if (v) return b << *v; return b.append("null", 4); };
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T>& b) { if (b) return s << *b; return s << "null"; }
template <class T, class U>
inline const bool operator==(const box<T>& l, const box<U>& r) { return (l && r && *l == *r) || (!l && !r); }
template <class T, class U>
inline const bool operator!=(const box<T>& l, const box<U>& r) { bool b = l, d = r; return !l && d || b && !r || (d && b && *l != *r); }
template <class T, class U>
inline const bool operator<(const box<T>& l, const box<U>& r) { return r && (!l || *l < *r); }
template <class T, class U>
inline const bool operator>(const box<T>& l, const box<U>& r) { return l && (!r || *l > *r); }
template <class T, class U>
inline const bool operator<=(const box<T>& l, const box<U>& r) { return !l || (r && *l <= *r); }
template <class T, class U>
inline const bool operator>=(const box<T>& l, const box<U>& r) { return !r || (l && *l >= *r); }
/// Compares an box to a `nullptr`
template <class T>
inline const bool operator==(const box<T>& l, std::nullptr_t) noexcept { return !l; }
template <class T>
inline const bool operator==(std::nullptr_t, const box<T>& r) noexcept { return !r; }
template <class T>
inline const bool operator!=(const box<T>& l, std::nullptr_t) noexcept { return l; }
template <class T>
inline const bool operator!=(std::nullptr_t, const box<T>& r) noexcept { return r; }
template <class T>
inline const bool operator<(const box<T>&, std::nullptr_t) noexcept { return false; }
template <class T>
inline const bool operator<(std::nullptr_t, const box<T>& r) noexcept { return r; }
template <class T>
inline const bool operator<=(const box<T>& l, std::nullptr_t) noexcept { return !l; }
template <class T>
inline const bool operator<=(std::nullptr_t, const box<T>&) noexcept { return true; }
template <class T>
inline const bool operator>(const box<T>& l, std::nullptr_t) noexcept { return l; }
template <class T>
inline const bool operator>(std::nullptr_t, const box<T>&) noexcept { return false; }
template <class T>
inline const bool operator>=(const box<T>&, std::nullptr_t) noexcept { return true; }
template <class T>
inline const bool operator>=(std::nullptr_t, const box<T>& r) noexcept { return !r; }
/// Compares the box with a value.
template <class T, class U>
inline const bool operator==(const box<T>& l, const U& r) { return l ? *l == r : false; }
template <class T, class U>
inline const bool operator==(const U& l, const box<T>& r) { return r ? l == *r : false; }
template <class T, class U>
inline const bool operator!=(const box<T>& l, const U& r) { return l ? *l != r : true; }
template <class T, class U>
inline const bool operator!=(const U& l, const box<T>& r) { return r ? l != *r : true; }
template <class T, class U>
inline const bool operator<(const box<T>& l, const U& r) { return l ? *l < r : true; }
template <class T, class U>
inline const bool operator<(const U& l, const box<T>& r) { return r ? l < *r : false; }
template <class T, class U>
inline const bool operator<=(const box<T>& l, const U& r) { return l ? *l <= r : true; }
template <class T, class U>
inline const bool operator<=(const U& l, const box<T>& r) { return r ? l <= *r : false; }
template <class T, class U>
inline const bool operator>(const box<T>& l, const U& r) { return l ? *l > r : false; }
template <class T, class U>
inline const bool operator>(const U& l, const box<T>& r) { return r ? l > *r : true; }
template <class T, class U>
inline const bool operator>=(const box<T>& l, const U& r) { return l ? *l >= r : false; }
template <class T, class U>
inline const bool operator>=(const U& l, const box<T>& r) { return r ? l >= *r : true; }
#undef _FORCE_INLINE
#endif // BOX_HPP