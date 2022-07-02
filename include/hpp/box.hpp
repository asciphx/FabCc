#ifndef BOX_H
#define BOX_H
#include <string>
#include <iostream>
#include <memory>
template <typename T>
class box {
  T* e;
  friend std::ostream& operator<<(std::ostream& s, box<T>& c);
  friend std::ostream& operator<<(std::ostream& s, const box<T>& c);
  friend std::string& operator<<(std::string& s, box<T>* c) {
	c->e == nullptr ? s += "null" : s << c->e; return s;
  };
  friend std::string& operator<<(std::string& s, const box<T>* c) {
	c->e == nullptr ? s += "null" : s << c->e; return s;
  };
  friend std::string& operator<<(std::string& s, box<T>& c) {
	c.e == nullptr ? s += "null" : s << c.e; return s;
  };
  friend std::string& operator<<(std::string& s, const box<T>& c) {
	c.e == nullptr ? s += "null" : s << c.e; return s;
  };
public:
  box(): e(nullptr) {}
  box(std::nullptr_t):e(nullptr) {}
  box(T&& _): e(new T(_)) {}
  ~box() { if (e != nullptr)delete e, e = nullptr; }
  void operator = (T&& s) { if (e != nullptr)delete e; e = new T(std::move(s)); }
  void operator = (T& s) { if (e != nullptr)delete e; e = new T(s); }
  T& operator() () { return *e; }
  const T& operator() ()const { return *e; }
  const T* $() const { return e; }
  void clear(){ if (e != nullptr)delete e, e = nullptr; }
};
template <typename T>
std::ostream& operator<<(std::ostream& s, box<T>& c) { return s << (c.e == nullptr ? "null" : *c.e); }
template <typename T>
std::ostream& operator<<(std::ostream& s, const box<T>& c) { return s << (c.e == nullptr ? "null" : *c.e); }

template<class T>
struct is_box: std::false_type {};
template<typename T>
struct is_box<box<T>>: std::true_type {};
template<typename T> struct box_pack {};
template<typename T> struct box_pack<box<T>> { using type = T; };
template<typename T> using box_pack_t = typename box_pack<T>::type;
#endif // BOX_H
