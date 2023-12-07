#include <iostream>
#include <string.h>
//#include <sys/types.h>
#include <thread>
#include <hh/client.hh>
int main() {
#ifdef _WIN32
  SetConsoleOutputCP(65001); setlocale(LC_CTYPE, ".UTF8"); WSADATA w; int l = WSAStartup(MAKEWORD(2, 2), &w);
  if (l != 0) { printf("WSAStartup failed with error: %d\n", l); return 1; }
#endif
  fc::client c("github.com");
  std::string s = c.get("/");
  for (auto& m : c.headers) {
    std::cout << m.first << ':' << m.second << ',' << '\n';
  }
  std::cout << s << '\n';
  return 0;
}
