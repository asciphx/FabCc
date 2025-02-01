#include <list>
#include <utility>
#include <memory>
#include <algorithm>
#include <fstream>
#include <mutex>
#include "app.hh"
#include "hpp/i2a.hpp"
#include "hh/lexical_cast.hh"
#include "hpp/string_view.hpp"
#include "h/common.h"
#include "h/any_types.h"
#include "hh/directory.hh"
#include "hh/http_error.hh"
#include "h/llhttp.h"
#include "hh/picohttpparser.hh"
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"
#endif
#ifndef _LLHTTP
#define _LLHTTP 0
#endif
#ifdef _MSVC_LANG
#define $_(_) _._Ptr
#else
#define $_(_) _.base()
#endif // _WIN32
namespace fc {
  HTTP c2m(const char* m, size_t l) {
    switch (hack_str(m, l)) {
    case 75064423044165:return HTTP::DEL;
    case 'GET':return HTTP::GET;
    case 'HEAD':return HTTP::HEAD;
    case 'POST':return HTTP::POST;
    case 'PUT':return HTTP::PUT;
    case 22324846097550931:return HTTP::OPTIONS;
    case 344693424968:return HTTP::PATCH;
    case 18946021178819412:return HTTP::CONNECT;
    } return HTTP::INVALID;
  }
  const std::string_view m2c(HTTP m) {
    switch (m) {
    case HTTP::DEL:return std::string_view("DELETE", 6);
    case HTTP::GET:return std::string_view("GET", 3);
    case HTTP::HEAD:return std::string_view("HEAD", 4);
    case HTTP::POST:return std::string_view("POST", 4);
    case HTTP::PUT:return std::string_view("PUT", 3);
    case HTTP::OPTIONS:return std::string_view("OPTIONS", 7);
    case HTTP::PATCH:return std::string_view("PATCH", 5);
    case HTTP::CONNECT:return std::string_view("CONNECT", 7);
    case HTTP::INVALID:return std::string_view("INVALID", 7);
    }
    return std::string_view("NULL", 4);
  }
  App::App(){}
  std::string App::get_cache(std::string& u) { if (RES_CACHE_TIME[u] > nowStamp()) return RES_CACHE_MENU[u]; return std::string(); };
  void App::set_cache(std::string& u, std::string& v, short i) { RES_CACHE_TIME[u] = nowStamp(i); RES_CACHE_MENU[u] = std::move(v); };
  App& App::set_file_download(bool&& b) { this->file_download = std::move(b); return *this; }
  VH& App::operator[](const char* r) {
    size_t l = strlen(r); if(!l) r = "/";
    if (fc::upload_path_.size() == l && !strncmp(r + 1, fc::upload_path_.c_str(), fc::upload_path_.size() - 1))
      printf("occupies the upload_path! line:%d " __FILE__, __LINE__), exit(0); VH& h = map_.add(r, static_cast<char>(HTTP::GET)); return h;
  }
  VH& App::del(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::DEL)); return h; }
  VH& App::get(const char* r) {
    size_t l = strlen(r); if(!l) r = "/";
    if (fc::upload_path_.size() == l && !strncmp(r + 1, fc::upload_path_.c_str(), fc::upload_path_.size() - 1))
      printf("occupies the upload_path! line:%d " __FILE__, __LINE__), exit(0); VH& h = map_.add(r, static_cast<char>(HTTP::GET)); return h;
  }
  VH& App::post(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::POST)); return h; }
  VH& App::put(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::PUT)); return h; }
  VH& App::patch(const char* r) { VH& h = map_.add(r, static_cast<char>(HTTP::PATCH)); return h; }
  //Set the file size allowed for POST according to the host memory
  //for example, 2G memory can be set to 512MB, because the idle memory is about 600+MB
  App& App::set_use_max_mem(const float& v) { USE_MAX_MEM_SIZE_MB = v <= 0x1ff ? 0x200 : v; return *this; }// < 600

  //template <typename Adaptor> //websocket
  //void handle_upgrade(Req& req, Res& res, Adaptor&& adaptor) { handle_upgrade(req, res, adaptor); }
  ///Process the Req and generate a Res for it
  std::string App::_print_routes() {
    std::string b(0xe0, '\0'); int i = 0; char m; b.clear();
#ifdef __linux__
    std::list<std::pair<std::string, VH>> aws;
    map_.for_all_routes([this, &aws](std::string r, VH h) { aws.push_front(std::make_pair(r, h)); });
    for (std::pair<std::string, VH>& p : aws) {
      std::string& r = p.first; VH& h = p.second; ++i;
      m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
      b << '(' << i << ')' << '[' << m2c((HTTP)m) << ']' <<
        '/' << (r[2] == 0x2f ? r.substr(3) : r.substr(2)) << (i % 6 == 0 ? '\n' : ' ') << ',';
    }
#else
    map_.for_all_routes([this, &b, &i, &m](std::string r, VH h) {
      m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210; ++i;
      b << '(' << i << ')' << '[' << m2c((HTTP)m) << ']' <<
        '/' << (r[2] == 0x2f ? r.substr(3) : r.substr(2)) << (i % 6 == 0 ? '\n' : ' ') << ',';
      });
#endif // __linux__
    b.pop_back(); return b;
  }
  App& App::file_type(const std::vector<std::string_view>& line) {
    for (std::vector<std::string_view>::const_iterator iter = line.cbegin(); iter != line.cend(); ++iter) {
      std::string_view sv; sv = content_any_types[*iter];
      if (sv != "") { content_types.emplace(*iter, sv); } else { content_types.emplace(*iter, RES_oct); }
    } return *this;
  }
  _CTX_TASK(void) App::_call(char m, std::string& r, Req& req, Res& res) const {
    //if (r[r.size() - 1] == '/') r = r.substr(0, r.size() - 1);// skip the last / of the url.
    //std::string g; static_cast<char>(m) < '\12' ? g.push_back(static_cast<char>(m) + 0x30) :
    //  (g.push_back(static_cast<char>(m) % 10 + 0x30), g.push_back(static_cast<char>(m) / 10 + 0x30)); g += r;
    std::string g(1, m + 0x30); g.append(r.data(), r.size());// std::cout << m2c(static_cast<HTTP>(m)) << ":" << r << "\n";
    fc::drt_node::iterator it = map_.root.find(g, 0); if (it.second != nullptr) {
      res.mask_url = std::move(g); co_await it->second(req, res);
    } else { res.mask_url = std::string("@", 1); co_await this->_.operator()(req, res); } co_return;
  }
  App& App::sub_api(const char* prefix, const App& app) {
    char m; if (prefix[0] == '/' || prefix[0] == '\\')++prefix;
    app.map_.for_all_routes([this, &prefix, &m](std::string r, VH h) {
      std::string $(1,'/'); if(*prefix)$ += prefix, $.push_back('/'); m = r[1] == 0x2f ? r[0] - 0x30 : r[0] * 10 + r[1] - 0x210;
      $ += r[1] == 0x2f ? r.substr(2) : r.substr(3); this->map_.add($.c_str(), m) = h;
      }); return *this;
  }
  App App::serve_file(const char* r = STATIC_DIRECTORY) {
    App app;
    try {
      char real_root[CROSSPLATFORM_MAX_PATH]{ 0 };
      if (r[0] == 0)r = "."; if (r[0] == '/' || r[0] == '\\')++r;
      if (!fc::crossplatform_realpath(r, real_root)) {
        std::string es("serve_file error: Directory ", 28); throw err::not_found(es << r << " does not exists.");
      }
      if (!fc::is_directory(real_root)) {
        std::string es("serve_file error: ", 18); throw err::internal_server_error(es << real_root << " is not a directory.");
      }
      std::string $(r); if ($.back() != '\\' && $.back() != '/') $.push_back('/'); fc::directory_ = $;
#ifndef __linux__
      app.map_.add("/", static_cast<char>(HTTP::GET)) = [$, this](Req& req, Res& res)_ctx{
        std::string _($); reinterpret_cast<Ctx*&>(res)->set_content_type("text/html;charset=UTF-8", 23);
        *reinterpret_cast<int*>(&req) = 1; _.append("index.html", 10);
        *reinterpret_cast<std::string*>(reinterpret_cast<char*>(&res) + _PTR_LEN) = std::move(_); co_return;
      };
#endif // !__linux__
      app.map_.add("/*", static_cast<char>(HTTP::GET)) = [$, this](Req& req, Res& res)_ctx{
        std::string _($); _.append(req.url.data() + 1, req.url.size() - 1);
        std::string::iterator i = _.end() - 1; if (*--i == '.')goto _; if (*--i == '.')goto _;
        if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
        if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
        res.mask_url = std::string("?", 1); co_await this->_.operator()(req, res); _CTX_return
      _ : std::size_t last_dot = $_(i) - $_(_.begin()) + 1;
        if (last_dot) {
          std::string ss{ toLowerCase(_.substr(last_dot)) }; std::string_view extension(ss.data(), ss.size());
          if (content_types.find(extension) != content_types.end()) {
            Ctx*& ctx = reinterpret_cast<Ctx*&>(res); std::string_view& sv{ content_types.at(extension) }; ctx->content_type = sv;
            u16 n = ++req.fiber.rpg->idx;
            if (extension[0] == 'h' && extension[1] == 't') {
              *reinterpret_cast<int*>(&req) = 1; *reinterpret_cast<std::string*>(reinterpret_cast<char*>(&res) + _PTR_LEN) = std::move(_);//maybe with zlib
            } else {
              *reinterpret_cast<int*>(&req) = 2; std::unordered_map<std::string, std::shared_ptr<fc::file_sptr>>::iterator p;
#ifdef __MINGW32__
              int path_len = ::MultiByteToWideChar(CP_UTF8, 0, _.c_str(), -1, NULL, 0);
              WCHAR* pwsz = new WCHAR[path_len]; ::MultiByteToWideChar(CP_UTF8, 0, _.c_str(), -1, pwsz, path_len);
              struct stat64 statbuf_; if (_wstat64(pwsz, &statbuf_) != 0) {
                delete[] pwsz; ctx->content_type = RES_NIL; pwsz = null; p = file_cache_.find(_);
                if(p != file_cache_.cend())p->second = std::make_shared<file_sptr>(); throw err::not_found();
              } delete[] pwsz; pwsz = null;
#else
              struct stat64 statbuf_; if (stat64(_.c_str(), &statbuf_) != 0) {
                p = file_cache_.find(_); if(p != file_cache_.cend())p->second = std::make_shared<file_sptr>();
                ctx->content_type = RES_NIL; throw err::not_found();
              }
#endif
              if (sv[2] == 'd' && sv[4] == 'o' && (sv[3] == 'i' || sv[1] == 'i')) {
                std::string range{ req.headers.operator[]("range") };
                if (!range.empty()) {
                  res.set_status(206); ctx->format_top_headers();
                  i64 l = range.find('=') + 1, r = range.rfind('-'); _Fsize_t pos = std::lexical_cast<_Fsize_t>(range.substr(l, r - l));
                  ctx->ot.append("Accept-Ranges: bytes\r\n", 22); range = range.substr(++r);
                  l = std::lexical_cast<long long>(range); r = l > 1 && l < statbuf_.st_size ? l : statbuf_.st_size - 1;
                  (ctx->ot.append("Content-Range: bytes ", 21u) << pos << '-' << r << '/' << statbuf_.st_size).append("\r\n", 2);
#ifdef _WIN32
                  //Because the windows system does not have a sendfile method, if > 4GB, need mmap
                  if (statbuf_.st_size > 0x100000000) { throw err::not_extended("windows is not extended!"); }
#endif
                  p = file_cache_.find(_);
                  if (p != file_cache_.cend() && p->second->modified_time_ == statbuf_.st_mtime) {
                    std::shared_ptr<file_sptr> ptr = p->second->shared_from_this(); co_await ctx->send_file(ptr, pos, ++r); _CTX_return
                  } else {
                    co_await ctx->send_file(file_cache_[_] = std::make_shared<file_sptr>(_, static_cast<_Fsize_t>(statbuf_.st_size), statbuf_.st_mtime), pos, ++r); _CTX_return
                  }
                }
                ctx->format_top_headers(); p = file_cache_.find(_);
                if (p != file_cache_.cend() && p->second->modified_time_ == statbuf_.st_mtime) {
                  std::shared_ptr<file_sptr> ptr = p->second->shared_from_this(); co_await ctx->send_file(ptr, this->file_download); _CTX_return
                } else {
                  co_await ctx->send_file(file_cache_[_] = std::make_shared<file_sptr>(_, static_cast<_Fsize_t>(statbuf_.st_size), statbuf_.st_mtime), this->file_download); _CTX_return
                }
                _CTX_return
              } //Non-media formats will only use the strategy of caching for one week
              ctx->format_top_headers(); ctx->ot.append("Cache-Control: max-age=604800,immutable\r\n", 41);
              p = file_cache_.find(_);
              if (p != file_cache_.cend() && p->second->modified_time_ == statbuf_.st_mtime) {
                std::shared_ptr<file_sptr> ptr = p->second->shared_from_this(); co_await ctx->send_file(ptr); _CTX_return
              } else {
                co_await ctx->send_file(file_cache_[_] = std::make_shared<file_sptr>(_, static_cast<_Fsize_t>(statbuf_.st_size), statbuf_.st_mtime)); _CTX_return
              }
            }//0.77 day ctx->ot.append("Cache-Control: " FILE_TIME"\r\n", 40);
            ROG* fib = req.fiber.rpg; req.fiber.timer.add_s(1, [n, fib] { if (fib->idx == n) { if (fib->_)fib->_.operator()(); } });
            _CTX_return
          }
          std::string es("Content-type of [", 17); throw err::not_found(es << extension << "] is not allowed!");
        }
        throw err::not_found();
      };
#ifdef __linux__
      app.map_.add("/", static_cast<char>(HTTP::GET)) = [$, this](Req& req, Res& res)_ctx{
        std::string _($); reinterpret_cast<Ctx*&>(res)->set_content_type("text/html;charset=UTF-8", 23);
        *reinterpret_cast<int*>(&req) = 1; _.append("index.html", 10);
        *reinterpret_cast<std::string*>(reinterpret_cast<char*>(&res) + _PTR_LEN) = std::move(_); co_return;
      };
#endif // __linux__
    } catch (const http_error& e) {
      printf("http_error[%d]: %s", e.i(), e.what());
    }
    return app;
  }
  App& App::set_keep_alive(int idle, int intvl, unsigned char probes) { k_A[0] = idle; k_A[1] = intvl; k_A[2] = probes; return *this; }
  struct llParser: public llhttp__internal_s {
    std::string& url; std::string_view& raw_url, header_field, body; fc::sv_map& headers; cc::query_string& url_params;
    llParser(std::string& u, std::string_view& a, fc::sv_map& h, cc::query_string& q): url(u), raw_url(a), headers(h), url_params(q) {}
  };
  static int on_url(llhttp__internal_s* _, const char* c, size_t s) {
    llParser* $ = static_cast<llParser*>(_); $->raw_url = DecodeURL(c, s);
    size_t l = $->raw_url.find('?'); if (l == -1) { $->url = std::string($->raw_url.data(), $->raw_url.size()); return 0; }
    $->url.clear(); $->url << $->raw_url.substr(0, l); $->url_params = cc::query_string($->raw_url, l); return 0;
  }
  static int on_header_field(llhttp__internal_s* _, const char* c, size_t l) {
    llParser* $ = static_cast<llParser*>(_); $->header_field = std::string_view(c, l); return 0;
  }
  static int on_header_value(llhttp__internal_s* _, const char* c, size_t l) {
    llParser* $ = static_cast<llParser*>(_); $->headers.emplace($->header_field, std::string_view(c, l)); return 0;
  }
  static int on_body(llhttp__internal_s* _, const char* c, size_t l) {
    llParser* $ = static_cast<llParser*>(_); $->body = std::string_view(c, l); return 0;
  }
  const static llhttp_settings_s RES_ll_ = { nullptr, on_url, nullptr, on_header_field, on_header_value, nullptr, on_body };

  App& App::set_ssl(std::string ciphers, std::string key, std::string cert) { ssl_key = key; ssl_cert = cert; ssl_ciphers = ciphers; return *this; }
#if __cplusplus < _cpp20_date
  static void make_http_processor(Conn& f, void* ap, Reactor * rc) {
#else
  fc::Task<void> make_http_processor(socket_type fd, sockaddr sa, int k, fc::timer & ft, ROG * re, epoll_handle_t eh, void* ap, Reactor * rc) {
    Conn f(fd, sa, k, ft, re, eh);
#if _OPENSSL
    if (rc->ssl_ctx && !f.ssl_handshake(rc->ssl_ctx)) { if (re->on) epoll_del_cpp20(eh, fd), re->on = 0; _CTX_return }
#endif
#endif
    fc::sv_map hd; cc::query_string up; std::string_view ru; std::string url; char rb[0x1000], wb[0x4000]; Ctx ctx(f, wb, sizeof(wb));
#if _LLHTTP
    llParser ll{ url, ru, hd, up }; llhttp__internal_init(&ll); ll.type = HTTP_REQUEST; ll.settings = (void*)&RES_ll_; int end = 0, r, last_len, pret;
#else
    const char* method, * path; size_t method_len, path_len; int end = 0, r, last_len, pret;
#endif
// #ifdef _WIN32
//     char id[10]; id[u2a(id, f.socket_fd) - id] = 0;
// #else
//     char id[11]; id[i2a(id, f.socket_fd) - id] = 0;
// #endif // _WIN32
    do {
      if (!(r = co_await f.read(rb, static_cast<int>(sizeof(rb))))) { _CTX_return } last_len = end; end += r;
      do {
#if _LLHTTP
        if (end == static_cast<int>(sizeof(rb))) { _CTX_return } pret = llhttp__internal_execute(&ll, rb + last_len, rb + r);
        if (pret == llhttp_errno::HPE_OK) {
          break;
        } else if (pret > 20) {
          last_len = end; end += (r = co_await f.read(rb + end, static_cast<int>(sizeof(rb) - end))); if (0 == r) { _CTX_return }
        } else { _CTX_return }
#else
        if (end == static_cast<int>(sizeof(rb))) { _CTX_return }
        pret = phr_parse_request(rb, end, &method, &method_len, &path, &path_len, &ctx.http_minor, &hd, &ctx.content_length_, last_len);
        if (pret > 0) {
          break;
          // const char* cur = rb + (*rb == 71 ? r - 1 : r >> 3);
          // do {
          //   switch (*cur) {
          //   case '\n':
          //     if (*(cur - 3) == '\r' && *(cur - 2) == '\n') { if (*(cur - 1) == '\r') { ++cur; pret = cur - rb; goto __; } ++cur; continue; }
          //     if (*(cur + 2) == '\n' && *(cur + 1) == '\r') { if (*(cur - 1) == '\r') { cur += 3; pret = cur - rb; goto __; } cur += 3; continue; }
          //   case '\r':
          //     if (*(cur - 2) == '\r' && *(cur - 1) == '\n') { if (*(cur + 1) == '\n') { cur += 2; pret = cur - rb; goto __; } cur += 2; continue; }
          //     if (*(cur + 3) == '\n' && *(cur + 2) == '\r' && *(cur + 1) == '\n') { cur += 4; pret = cur - rb; goto __; }
          //   default:cur += 4;
          //   }
          // } while (cur - rb < end); pret = end;
        } else if (pret == -1) {
          _CTX_return;
        } else if (pret == -2) {
          last_len = end; end += (r = co_await f.read(rb + end, static_cast<int>(sizeof(rb) - end))); if (0 == r) { _CTX_return }
        }
#endif
      } while (RESon);
#ifdef _WIN32
      f.epoll_mod(EPOLLOUT | EPOLLRDHUP);
#endif // _WIN32
#if _LLHTTP
      Req req{ static_cast<HTTP>(ll.method), url, ru, hd, up, f, static_cast<App*>(ap)->USE_MAX_MEM_SIZE_MB, ctx.cookie_map, ctx.cache_file };
      ctx.content_length_ = ll.content_length; req.body = ll.body; ctx.http_minor = ll.http_minor; Res res(ctx, static_cast<App*>(ap));
      std::string* res_body = reinterpret_cast<std::string*>(reinterpret_cast<char*>(&res) + _PTR_LEN);
      if (ll.finish) {
#else
      ru = DecodeURL(path, path_len); path_len = ru.find('?');
      if (path_len == -1) { url = std::string(ru.data(), ru.size()); } else {
        url.clear(); url << ru.substr(0, path_len); up = cc::query_string(ru, path_len);
      }
      Req req{ c2m(method, method_len), url, ru, hd, up, f, static_cast<App*>(ap)->USE_MAX_MEM_SIZE_MB, ctx.cookie_map, ctx.cache_file };
      req.body = std::string_view(rb + pret, end - pret); Res res(ctx, static_cast<App*>(ap));
      std::string* res_body = reinterpret_cast<std::string*>(reinterpret_cast<char*>(&res) + _PTR_LEN);
      if (end == pret && ctx.content_length_) {
#endif
        int n = ++f.rpg->idx; f.is_idle = false;
        try {
          req.length = std::move(ctx.content_length_); co_await static_cast<App*>(ap)->_call(static_cast<char>(req.method), url, req, res);
          ctx.format_top_headers(); ctx.respond(res_body->size(), res.headers); f.is_idle = true;
#if _LLHTTP
          llhttp_reset(&ll);
#endif
        } catch (const http_error& e) {
          ctx.set_status(e.i()); *res_body = e.what(); ctx.ot.reset(); ctx.format_top_headers(); ctx.respond(res_body->size(), res.headers);
        } catch (const std::exception& e) {
          ctx.set_status(500); *res_body = e.what(); ctx.ot.reset(); ctx.format_top_headers(); ctx.respond(res_body->size(), res.headers);
        }
        ctx.prepare_next_request(); end = 0; hd.clear(); up.clear();
        co_await ctx.ot.flush(std::move(*res_body)); time(&f.rpg->hrt);
        ROG* fib = f.rpg; f.timer.add_s(f.k_a - 1, [n, fib] { if (fib->idx == n && fib->_) { fib->_.operator()(); } });
        continue;
      }
      try {
        co_await static_cast<App*>(ap)->_call(static_cast<char>(req.method), url, req, res);
        switch (*reinterpret_cast<int*>(&req)) {
        case 1:
        {
          int64_t& ct_value = RES_CACHE_TIME[url]; bool ct_check = ct_value > nowStamp(); unsigned int l = 0; ctx.format_top_headers();
          if (ct_check) {
            std::string& bo = RES_CACHE_MENU[url]; l = static_cast<unsigned int>(bo.size()); ctx.prepare_next_request();
            ctx.ot.append("Access-Control-Allow-origin: *\r\n", 32) << RES_CE << RES_seperator << RES_gzip
              << RES_crlf << RES_content_length_tag << l << RES_crlf; end = 0; hd.clear(); up.clear();
            co_await ctx.ot.append("Content-Type: text/html;charset=UTF-8", 37).append("\r\n\r\n", 4).flush(bo);
#ifndef _WIN32
#if __cplusplus < _cpp20_date
            f.rpg->_.operator()();
#else
            co_await std::suspend_always{};
#endif
#endif // !_WIN32
            continue;
          } else {
  #ifdef __MINGW32__
            int path_len = ::MultiByteToWideChar(CP_UTF8, 0, res_body->c_str(), -1, NULL, 0);
            WCHAR* pwsz = new WCHAR[path_len]; ::MultiByteToWideChar(CP_UTF8, 0, res_body->c_str(), -1, pwsz, path_len);
            if ((*reinterpret_cast<int*>(&req) = _wstat64(pwsz, &rc->statbuf_)) != 0) {
              delete[] pwsz; ctx.content_type = RES_NIL; pwsz = null; throw err::not_found(*res_body << " -> Not Found!");
            } delete[] pwsz; pwsz = null;
  #else
            if ((*reinterpret_cast<int*>(&req) = stat(res_body->c_str(), &rc->statbuf_)) != 0) {
              ctx.content_type = RES_NIL; throw err::not_found(*res_body << " -> Not Found!");
            }
  #endif
            std::ifstream inf(*res_body, std::ios::in | std::ios::binary);// if (!inf.is_open()) { throw err::not_found(*res_body << " -> Not Found!"); }
            res_body->clear(); inf.seekg(0, std::ios_base::end); auto file_size = inf.tellg();//default 600kb for html's max size
            if (file_size >> 10 > static_cast<App*>(ap)->USE_MAX_MEM_SIZE_MB) { inf.close(); throw err::forbidden("Html file too large!"); }
            inf.seekg(0, std::ios_base::beg); std::stringstream fb; fb << inf.rdbuf(); std::string bo = std::move(fb.str());
            bo = res.compress_str((char*)bo.data(), (u32)bo.size()); inf.close(); l = static_cast<unsigned int>(bo.size()); bo.shrink_to_fit();
            ct_value = nowStamp(CACHE_HTML_TIME_SECOND); RES_CACHE_MENU[url] = bo; ctx.prepare_next_request();
            ctx.ot.append("Access-Control-Allow-origin: *\r\n", 32) << RES_CE << RES_seperator
              << RES_gzip << RES_crlf << RES_content_length_tag << l << RES_crlf; end = 0; hd.clear(); up.clear();
            co_await ctx.ot.append("Content-Type: text/html;charset=UTF-8", 37).append("\r\n\r\n", 4).flush(std::move(bo));
#ifndef _WIN32
#if __cplusplus < _cpp20_date
            f.rpg->_.operator()();
#else
            co_await std::suspend_always{};
#endif
#endif // !_WIN32
            continue;
          }
        }
        break;
        case 2: *reinterpret_cast<int*>(&req) = 0; break;
        default: ctx.format_top_headers();
          // ctx.add_header("ip", req.ip_address().c_str()); ctx.add_header("id", id);
          ctx.respond(res_body->size(), res.headers);
        }
      } catch (const http_error& e) {// If error code is equal 500, record the log
        ctx.set_status(e.i()); *res_body = e.what(); ctx.ot.reset(); ctx.format_top_headers(); ctx.respond(res_body->size(), res.headers);
        if (e.i() == 500) std::cout << "ERROR<" << e.i() << ">: " << e.what() << std::endl;
      } catch (const std::exception& e) {//No logging is required as log takes up disk space
        ctx.set_status(500); *res_body = e.what(); ctx.ot.reset(); ctx.format_top_headers(); ctx.respond(res_body->size(), res.headers);
      }
      ctx.prepare_next_request(); end = 0; hd.clear(); up.clear(); co_await ctx.ot.flush(std::move(*res_body));
    } while (RESon);
    co_return;
  }
  void App::http_serve(int port, std::string ip, int nthreads) {
    if (!fc::is_directory(fc::directory_)) fc::create_directory(fc::directory_);
    if (!fc::is_directory(fc::directory_ + fc::upload_path_)) fc::create_directory(fc::directory_ + fc::upload_path_);
    if (!fc::is_directory("_/")) fc::create_directory("_/"); RES_START_TIME = std::chrono::high_resolution_clock::now();
    for (int i = -1; i < 0xff;) { char c[4]; snprintf(c, 4, "%02x", ++i); fc::create_directory(std::string("_/", 2) + c); }
#if _OPENSSL
    std::cout << "C++<web>[" << static_cast<int>(nthreads) << "] => ht" << (ssl_cert.size() ? "tps" : "tp") << "://127.0.0.1:" << port << std::endl;
#else
    std::cout << "C++<web>[" << static_cast<int>(nthreads) << "] => http://127.0.0.1:" << port << std::endl;
#endif
#ifdef _WIN32
    SetConsoleOutputCP(65001); setlocale(LC_CTYPE, ".UTF8"); WSADATA w; int err = WSAStartup(MAKEWORD(2, 2), &w);
    if (err != 0) { std::cerr << "WSAStartup failed with error: " << err << std::endl; return; } // Setup quit signals
    signal(SIGINT, shutdown_handler); signal(SIGTERM, shutdown_handler); signal(SIGABRT, shutdown_handler);
    std::thread date_thread([]() { while (RESquit_signal_catched) { fc::REStop_h.tick(), std::this_thread::sleep_for(std::chrono::milliseconds(1)); } });
#else
    struct sigaction act; memset(&act, 0, sizeof(act)); act.sa_handler = shutdown_handler;
    sigaction(SIGINT, &act, 0); sigaction(SIGTERM, &act, 0); sigaction(SIGQUIT, &act, 0);
    // Ignore sigpipe signal. Otherwise sendfile causes crashes if the
    // client closes the connection during the response transfer.
    std::thread date_thread([]() { while (RESquit_signal_catched) { fc::REStop_h.tick(), std::this_thread::sleep_for(std::chrono::milliseconds(4)); } });
#endif
#if __APPLE__ || __linux__
    signal(SIGPIPE, SIG_IGN);
#endif
    // Start the server threads.
    const char* listen_ip = !ip.empty() ? ip.c_str() : nullptr;
    socket_type sfd = create_and_bind(listen_ip, port, SOCK_STREAM); if (sfd == (socket_type)EOF) return;
#ifdef __linux__
    struct linger lll { 1, 0 }; setsockopt(sfd, SOL_SOCKET, SO_LINGER, &lll, sizeof(struct linger));
#endif
    setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE,
#if _WIN32
    (const char*)&RESkeep_AI
#else
      & RESkeep_AI
#endif
      , sizeof(RESkeep_AI));
    start_server(date_thread, sfd, nthreads, make_http_processor, k_A, this, ssl_key, ssl_cert, ssl_ciphers);//SOCK_DGRAM
  }
} // namespace fc
#if __clang__
#pragma clang diagnostic pop
#endif