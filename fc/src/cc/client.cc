#include <hh/client.hh>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <hpp/i2a.hpp>
#include "openssl/rand.h"
#if defined(__MINGW32__)
static void inet_pton(int af, const char* src, void* dst) {
  struct sockaddr_in ip4; struct sockaddr_in6 ip6; char ipaddr[64]; sprintf(ipaddr, "%s:2020", src);
  int as = -1; if (af == AF_INET) {
    as = sizeof(SOCKADDR_IN); WSAStringToAddressA((LPSTR)ipaddr, af, NULL, (LPSOCKADDR)&ip4, &as); memcpy(dst, &(ip4.sin_addr), 4);
  } else if (af == AF_INET6) {
    as = sizeof(SOCKADDR_IN6); WSAStringToAddressA((LPSTR)ipaddr, af, NULL, (LPSOCKADDR)&ip6, &as); memcpy(dst, &(ip6.sin6_addr), 16);
  }
}
#endif
namespace fc {
  const char* m2c(HTTPS m) {
    switch (m) {
    case HTTPS::DEL:return "DELETE ";
    case HTTPS::GET:return "GET ";
    case HTTPS::HEAD:return "HEAD ";
    case HTTPS::POST:return "POST ";
    case HTTPS::PUT:return "PUT ";
    case HTTPS::OPTIONS:return "OPTIONS ";
    case HTTPS::PATCH:return "PATCH ";
    case HTTPS::CONNECT:return "CONNECT ";
    }
    return "";
  }
  client::client(const char* i, u_short p, HTTPS h): _port(p), _ip(i), conn(true), l(0), r(3), method(h), body(512, '\0') {
    if (!openssl_init) { SSL_load_error_strings(); OpenSSL_add_ssl_algorithms(); openssl_init = true; }
#if defined _WIN32
    srand((unsigned)time(NULL)); for (int i = 0; i < 100; i++)  seed_int[i] = rand(); RAND_seed(seed_int, sizeof(seed_int));
    fd = (int)socket(AF_INET, SOCK_STREAM, 0); if (fd == -1) { WSACleanup(); conn = 0; }
#else
    fd = socket(AF_INET, SOCK_STREAM, 0); if (fd == -1) { conn = 0; }
#endif
    memset(&sa, 0, sizeof(sa)); sa.sin_family = AF_INET; sa.sin_port = htons(_port); inet_pton(AF_INET, _ip, (void*)&sa.sin_addr);
    int err = ::connect(fd, (struct sockaddr*)&sa, sizeof(sa)); if (0 != err) {
#if defined _WIN32
      closesocket(fd);
#else
      ::close(fd);
#endif
      conn = 0; ctx = SSL_CTX_new(TLS_client_method()); cctx = SSL_CONF_CTX_new();
      SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_CLIENT); SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
      SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL); sbio = BIO_new_ssl_connect(ctx); if (_port == 80) _port = 443;
      BIO_get_ssl(sbio, &ssl); char t[5]; t2a(t, _port); body.clear(); body.append(_ip).append({ ':' }).append(t);
      BIO_set_conn_hostname(sbio, body.data()); if (BIO_do_connect(sbio) <= 0) {
        fprintf(stderr, "Err!\n"); SSL_free(ssl); SSL_CTX_free(ctx); SSL_CONF_CTX_free(cctx); BIO_free_all(sbio);
      } else conn = 1;
    } else conn = 2;
  }
  std::string& client::get(const std::string& url, const std::string& body, int n) {
    std::string wb = m2c(method); wb += url; this->body.clear(); if (header[0]) {
      wb.append(" HTTP/1.1\r\n", 11).append("Host: ", 6).append(_ip).append("\r\n", 2); wb += header; wb.append("\r\n", 2);
    } else {
      wb.append(" HTTP/1.1\r\n", 11).append("Host: ", 6).append(_ip).append("\r\n\r\n", 4);
    } wb += body;
    if (conn == 1) {
      r = SSL_write(ssl, wb.data(), (int)wb.size()); if (r > 0) l += r;
      while (l > 0 && l < wb.size() && --n) {
        r = SSL_write(ssl, wb.data() + l, (int)wb.size() - l); if (r > 0) l += r;
        if (r == 0) return this->body;
      }
      r = SSL_read(ssl, rb, sizeof(rb));
      const char* cur = rb + 0XE; int of = l = r > 0 ? r : 0;
      while (conn) {
        do {
          switch (*cur) {
          case '\r':
            if (*(cur - 2) == '\r' && *(cur - 1) == '\n') {
              cur += 2; of = static_cast<int>(cur - rb); goto $;
            } cur += 2; break;
          case '\n':
            if (*(cur - 3) == '\r' && *(cur - 2) == '\n' && *(cur - 1) == '\r') {
              cur += 1; of = static_cast<int>(cur - rb); goto $;
            }
          default:cur += 2;
          }
        } while (static_cast<int>(cur - rb) <= l);
        r = SSL_read(ssl, rb + l, sizeof(rb) - l);
        while (r < 0 && --n) {
          r = SSL_read(ssl, rb + l, sizeof(rb) - l);
        }
        if (0 == r) { break; }
        l += r;
      }$:
      const char* path; size_t path_len, num = 0; int minor_version, status, flag = 0; _ssize_t content_length_;
      flag = phr_parse_response(rb, of, &minor_version, &status, &path, &path_len, &headers, &num, &content_length_, 0);
      if (content_length_ > 0) {
        int i = l - of; std::string_view sv(rb + of, i); if (i)this->body << sv;
        if (content_length_ > i) {
          this->body.resize(content_length_); n += (int)content_length_ >> 8;
          do {
            r = SSL_read(ssl, const_cast<char*>(this->body.data()) + i, static_cast<int>(content_length_ - i));
            while (r < 0 && --n) {
              r = SSL_read(ssl, const_cast<char*>(this->body.data()) + i, static_cast<int>(content_length_ - i));
            } i += r;
            if (r == 0) return this->body;
          } while (content_length_ > i);
        }
        return this->body;
      } else {
        this->body.resize(32768); int i = 0;
        do {
          if (r > 0) i += r;
        } while ((r = SSL_read(ssl, const_cast<char*>(this->body.data()) + i, static_cast<int>(this->body.size() - i))) != 0);
        if(i == this->body.size()) this->body.resize(this->body.size() << 1);
        do {
          if (r > 0) i += r;
        } while ((r = SSL_read(ssl, const_cast<char*>(this->body.data()) + i, static_cast<int>(this->body.size() - i))) != 0);
        if(i == this->body.size()) this->body.resize(this->body.size() << 1);
        do {
          if (r > 0) i += r;
        } while ((r = SSL_read(ssl, const_cast<char*>(this->body.data()) + i, static_cast<int>(this->body.size() - i))) != 0);
        if(i == this->body.size()) return this->body = "";
      }
    } else if (conn == 2) {
      r = send(fd, wb.data(), (int)wb.size(), 0); if (r > 0) l += r;
      while (l > 0 && l < wb.size() && --n) {
        r = send(fd, wb.data() + l, (int)wb.size() - l, 0); if (r > 0) l += r;
        if (r == 0) return this->body;
      }
      r = recv(fd, rb, sizeof(rb), 0);
      while (r <= 0 && --n) {
        r = recv(fd, rb + l, sizeof(rb) - l, 0);
      }
      const char* cur = rb + 0XE; int of = l = r > 0 ? r : 0;
      while (conn) {
        do {
          switch (*cur) {
          case '\r':
            if (*(cur - 2) == '\r' && *(cur - 1) == '\n') {
              cur += 2; of = static_cast<int>(cur - rb); goto _;
            } cur += 2; break;
          case '\n':
            if (*(cur - 3) == '\r' && *(cur - 2) == '\n' && *(cur - 1) == '\r') {
              cur += 1; of = static_cast<int>(cur - rb); goto _;
            }
          default:cur += 2;
          }
        } while (static_cast<int>(cur - rb) <= l);
        r = recv(fd, rb + l, sizeof(rb) - l, 0);
        while (r < 0 && --n) {
          r = recv(fd, rb + l, sizeof(rb) - l, 0);
        }
        if (0 == r) { break; }
        l += r;
      }_:
      const char* path; size_t path_len, num = 0; int minor_version, status, flag = 0; _ssize_t content_length_;
      flag = phr_parse_response(rb, of, &minor_version, &status, &path, &path_len, &headers, &num, &content_length_, 0);
      if (content_length_ > 0) {
        int i = l - of; std::string_view sv(rb + of, i); if (i)this->body << sv;
        if (content_length_ > i) {
          this->body.resize(content_length_); n += (int)content_length_ >> 8;
          do {
            r = recv(fd, const_cast<char*>(this->body.data()) + i, static_cast<int>(content_length_ - i), 0);
            while (r < 0 && --n) {
              r = recv(fd, const_cast<char*>(this->body.data()) + i, static_cast<int>(content_length_ - i), 0);
            } i += r;
            if (r == 0) return this->body;
          } while (content_length_ > i);
        }
        return this->body;
      }
    }
    return this->body;
  }
  client::~client() {
    if (conn == 1) {
      if (ssl) {
        SSL_CTX_free(ctx);
        SSL_CONF_CTX_free(cctx);
        BIO_free_all(sbio);
      }
    } else if (conn == 2) {
#if defined _WIN32
      closesocket(fd);
#else
      ::close(fd);
#endif
    }
  };
  int client::close() {
    if (conn == 1) {
      conn = 0;
      if (ssl) {
        SSL_shutdown(ssl);
        //SSL_free(ssl);
        SSL_CTX_free(ctx);
        SSL_CONF_CTX_free(cctx);
        BIO_free_all(sbio);
      }
    } else if (conn == 2) {
      conn = 0;
#if defined _WIN32
      return closesocket(fd);
#else
      return ::close(fd);
#endif
    }
    return 0;
  }
}