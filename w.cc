#include <lexical_cast.hh>
#include <hpp/text.hpp>
#include <time.h>
#include <iostream>
//#include <str_map.hh>
//#include <llhttp.h>
int main() {
  clock_t start = clock();
  unsigned long long l;
  // double d;
  std::string s("18446744073709551615", 20),
	num("18446744073709551615", 20);
  for (unsigned long long i = 5; i < 9672955; ++i) {
	// l = std::lexical_cast<int>("123456");
	// d = std::lexical_cast<double>("54345.5466");
	// s = atoi("543455466");
	l = std::lexical_cast<unsigned long long>(num);
  }
  printf("use %.6f seconds\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  l = std::lexical_cast<unsigned long long>(s);
  //printf("%.6lf\n", std::lexical_cast<double>("54345.5466"));
  //printf("%f\n", std::lexical_cast<double>("0xffffff.ff"));
  std::cout << l;
  fc::text<8> t("你好世界！我好！世界"); std::cout << t;
  return 0;
}
