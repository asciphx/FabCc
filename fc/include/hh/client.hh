#ifndef CLIENT_HH
#define CLIENT_HH
#if __linux__
#include <sys/epoll.h>
#elif __APPLE__
#include <sys/event.h>
#endif
#if defined __linux__ || defined __APPLE__
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#ifndef _WIN32
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#include <h/windows.h>
#define sleep(x) Sleep(x * 1000)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <locale.h>
//#pragma comment (lib, "PSAPI.lib")
//#pragma comment (lib, "ws2_32.lib")
//#pragma comment (lib, "wsock32.lib")
#endif
#include <hpp/string_view.hpp>
#include <hh/picohttpparser.hh>
#ifndef _OPENSSL
#define _OPENSSL 0
#endif
#if _OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif
namespace fc {
  enum HTTPS {
    DEL = 0, GET, HEAD, POST, PUT, OPTIONS, PATCH, CONNECT, INVALID
  };
  static bool openssl_init = false;
  struct client {
    client(const client&) = delete;
    void operator=(const client&) = delete;
    client(const char* ip = "127.0.0.1", u_short port = 80, HTTPS h = HTTPS::GET);
    ~client();
    //By default, reconnection will be attempted once every one second, with a total of 3 attempts.
    std::string& get(const std::string& url = "/", const std::string& body = "", int n = 6);
    void add_header(const char* k, const char* v) { header.append(k).append(": ", 2).append(v).append("\r\n", 2); }
    int close();
    fc::sv_map headers;
    std::string body;
  private:
    std::string header;
    sockaddr_in sa;
#if _OPENSSL
    BIO* sbio = NULL;
    SSL_CTX* ctx;
    SSL_CONF_CTX* cctx;
    SSL* ssl;
#endif
    int fd;
    int seed_int[100];
    int l, r, conn;
    char rb[0x1000] = { 0 };
    HTTPS method;
    const char* _ip;
    u_short _port;
  };
}
#endif
