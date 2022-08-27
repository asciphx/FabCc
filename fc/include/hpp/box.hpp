#ifndef BOX_H
#define BOX_H
#include <string>
#include <iostream>
#include <memory>
template <typename T>
class box {
  friend std::string& operator<<(std::string& s, box<T>* c) {
	c->p == nullptr ? s += "null" : s << c->p; return s;
  };
  friend std::string& operator<<(std::string& s, const box<T>* c) {
	c->p == nullptr ? s += "null" : s << c->p; return s;
  };
  friend std::string& operator<<(std::string& s, box<T>& c) {
	c.p == nullptr ? s += "null" : s << c.p; return s;
  };
  friend std::string& operator<<(std::string& s, const box<T>& c) {
	c.p == nullptr ? s += "null" : s << c.p; return s;
  };
  bool b = true;
public:
  T* p;
  box() noexcept: p(0) {}
  box(std::nullptr_t) noexcept: p(0) {}
  template<typename U>
  box(box<U>&& x) noexcept: p(x.p) { x.p = 0; }
  template<typename U>
  box(box<U>& x) noexcept: p(x.p) { x.p = 0; }
  box(T&& _): p(new T(_)) {}
  box(T& _): p(&_), b(false) {}
  template<typename... U>
  box(U... t) noexcept: p(new T{ t... }) {}
  ~box() { if (b)delete p, p = nullptr; }
  void operator = (T& s) { if (b)delete p; p = &s; b=false; }
  void operator = (T&& s) { if (b)delete p; p = new T(s); }
  T& operator() () { return *p; }
  const T& operator() ()const { return *p; }
};
template <typename T>
std::ostream& operator<<(std::ostream& s, box<T>& c) { return s << (c.p == nullptr ? "null" : *c.p); }
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T>& c) { return s << (c.p == nullptr ? "null" : *c.p); }

template<class T>
struct is_box: std::false_type {};
template<typename T>
struct is_box<box<T>>: std::true_type {};
template<typename T> struct box_pack {};
template<typename T> struct box_pack<box<T>> { using type = T; };
template<typename T> using box_pack_t = typename box_pack<T>::type;
#include <vector>
template<typename T>
using vec = std::vector<T>;
#endif // BOX_H
