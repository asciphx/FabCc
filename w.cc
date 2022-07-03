#include <lexical_cast.hh>
#include <time.h>
int main() {
  clock_t start = clock();
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
  return 0;
}
