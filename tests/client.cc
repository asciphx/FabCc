#include <iostream>
#include <string.h>
//#include <sys/types.h>
#include <thread>
#include <hh/client.hh>
int main(int argc, char* argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(65001); setlocale(LC_CTYPE, ".UTF8"); WSADATA w; int l = WSAStartup(MAKEWORD(2, 2), &w);
  if (l != 0) { printf("WSAStartup failed with error: %d\n", l); return 1; }
#endif
#if _OPENSSL
  fc::client c("github.com");
#else
  fc::client c("127.0.0.1", 8080);
#endif
  std::string url("/", 1);
  url += argc < 2 ? std::string("", 0) : argv[1];
  std::string& s = c.get(url);
  for (auto& m : c.headers) {
    std::cout << m.first << ':' << m.second << '\n';
  }
  std::cout << s << '@';
  return 0;
}