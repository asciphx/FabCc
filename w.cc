#include <lexical_cast.hh>
#include <char_map.hh>
#include <hpp/text.hpp>
#include <time.h>
#include <iostream>

int main() {
  clock_t start = clock(); cc::char_map map;
  std::string s, num("156453", 6);
  int l;
  for (int i = 0; i < 1000000; ++i) {
	s = std::lexical_cast<std::string>(i);
	l = std::lexical_cast<int>(num);
  }
  printf("%.6lf\n", std::lexical_cast<double>("54345.5466"));
  //printf("%f\n", std::lexical_cast<double>("0xffffff.ff"));
  printf("use %.6f seconds\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  text<8> t("你好世界！我好！世界"); std::cout << t;
  return 0;
}
