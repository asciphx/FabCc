#ifndef BOX_H
#define BOX_H
#include <string>
#include <iostream>
#include <memory>
template <typename T>
class box {
public:
  box() {}
  box(std::nullptr_t) {}
  box(T&& _) {}
  ~box() {}
  inline void operator = (T&& s);
  inline void operator = (T& s);
  inline T& operator() ();
  inline const T& operator() ()const;
  inline const T* $() const;
  inline void clear();
};
template<class T> struct is_box: std::false_type {};
template<typename T> struct is_box<box<T>>: std::true_type {};
template<typename T> struct box_pack {};
template<typename T> struct box_pack<box<T>> { using type = T; };
template<typename T> using box_pack_t = typename box_pack<T>::type;
#endif // BOX_H
