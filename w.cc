#include <lexical_cast.hh>
#include <char_map.hh>
#include <text.hpp>
#include <time.h>
#include <iostream>
template<unsigned short I>
struct Char {
  char _[I]; static inline unsigned short L = I - 1;
  Char(const char* c) {
	unsigned short i = 0; while (i < L && c[i]) _[++i] = c[i]; _[++i] = 0;
  }
  char& operator [](unsigned short i) {
	return _[i];
  }
  const char* c_str() {
	return _;
  }
};
Char<32> itoa(int32_t i, Char<32> p = "") {
  if (i < 0) {
	p[0] = '-';
  } p[1] = 0;
  return p;
}
int main() {
  clock_t start = clock(); cc::char_map map;
  std::string s;
  int l;
  for (int i = 0; i < 1000000; ++i) {
	s = std::lexical_cast<std::string>(i);
	l = std::lexical_cast<int>("156453");
  }
  printf("%.6lf\n", std::lexical_cast<double>("54345.5466"));
  //printf("%f\n", std::lexical_cast<double>("0xffffff.ff"));
  printf("use %.6f seconds\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  printf(itoa(-234).c_str());
  text<15> t("你好世界！");
  std::cout << t;
  return 0;
}
