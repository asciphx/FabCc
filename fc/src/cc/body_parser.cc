/*
* This software is licensed under the AGPL-3.0 License.
*
* Copyright (C) 2023 Asciphx
*
* Permissions of this strongest copyleft license are conditioned on making available
* complete source code of licensed works and modifications, which include larger works
* using a licensed work, under the same license. Copyright and license notices must be
* preserved. Contributors provide an express grant of patent rights. When a modified
* version is used to provide a service over a network, the complete source code of
* the modified version must be made available.
*/
#include <sstream>
#include <fstream>
#include <set>
#include <h/common.h>
#include <hh/directory.hh>
#include <hh/http_error.hh>
#include <hh/body_parser.hh>
#include <random>
namespace fc {
#ifdef WIN32
  float GetMemUsage(int pid) {
    uint64_t m = 0; PROCESS_MEMORY_COUNTERS pmc; HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc))) m = pmc.PagefileUsage; CloseHandle(h); return m / 1048576.0;
#else
  float GetMemUsage(int pid) {
    char file_name[64] = { 0 }, line_buff[512] = { 0 }; sprintf(file_name, "/proc/%d/status", pid); FILE* fd = fopen(file_name, "r");
    if (nullptr == fd) return 0; int v = 0; for (int i = 0; i < 22; ++i) char* cha = fgets(line_buff, sizeof(line_buff), fd);
    sscanf(line_buff, "%s %d", file_name, &v); fclose(fd); return v / 1024.0;
#endif
  }
  BP::BP(Req & req, const char* m, unsigned short mb): menu(fc::upload_path_), L(mb), req(req), ban_file(false),
    boundary(g_b(req.header(RES_CT))), content_length_(req.length) {
    menu += m; if (menu[menu.size() - 1] != '/')menu.push_back('/'); if (RES_menu.find(menu) == RES_menu.end()) {
      std::string ss(fc::directory_); ss += menu; RES_menu.insert(menu); if (!fc::is_directory(ss)) { fc::create_directory(ss); }
    }
  }
  BP::BP(Req & req, unsigned short mb, bool b): menu(fc::upload_path_), L(mb), req(req), ban_file(b),
    boundary(g_b(req.header(RES_CT))), content_length_(req.length) {}
  std::string_view BP::g_b(const std::string_view & h) const {
    //std::cout << "<" << h << ">" << h.size() << std::endl;
    size_t f = h.find("=----"); if (f != -1) return h.substr(f + 0xe); return h;//raw
  }
  _CTX_TASK(void) BP::run() {//std::cout<<boundary<<std::endl;
    std::string value(req.body.data(), req.body.size());
    if (content_length_) {
      if (ban_file) { req.fiber.shut(_READ); throw err::not_implemented("File not allowed!"); }
      if (content_length_ > L * 1048576ll) {
        req.fiber.shut(_READ); std::string es; throw err::too_large(es << "Body size can't be biger than : " << L << "MB");
      }
      float mem{ GetMemUsage() };//not finish read
      if (mem > req.USE_MAX_MEM_SIZE_MB) {
        req.fiber.shut(_READ); std::string es; throw err::internal_server_error(es << "insufficient memory!" << mem);
      }//More possibility to use memory. If the conditions are met in memory, there is no need to use disk.
      long long o = req.body.size();
      if (content_length_ < 65536 || (content_length_ >> 18) < req.USE_MAX_MEM_SIZE_MB - mem) {
        value.resize(content_length_);
        int N = co_await req.fiber.read(const_cast<char*>(value.data() + o), content_length_ - o); o += N > 0 ? N : 0;
        while (content_length_ > o) {
          if (N > 0) {
            N = co_await req.fiber.read(const_cast<char*>(value.data() + o), content_length_ - o); o += N;
          } else N = co_await req.fiber.read(const_cast<char*>(value.data() + o), content_length_ - o);
        }
        value.end() += content_length_; content_length_ = 0;
      } else {
        std::random_device rd; std::string mask("_/", 2); std::ostringstream os; os << std::hex << std::setw(4) << rd();
        std::string ss{ os.str() }; mask.append({ ss[3], ss[2], '/', ss[1] }) << (std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - RES_START_TIME).count() - ss[2] * ss[3] - ss[1]) << ss[0];
        req.cache_file = std::unique_ptr<cache_file>(new cache_file{ mask.data(), mask.size() });
        value.resize(65536); int N = co_await req.fiber.read(const_cast<char*>(value.data() + o), 65536 - o); N += o;
        do {
          if (N > 0) const_cast<Req&>(req).cache_file->append(const_cast<char*>(value.data()), N), o += N;
        } while (content_length_ > o && (N = co_await req.fiber.read(const_cast<char*>(value.data()), 65536)));
      }
    } else {
      std::string es;
      if (value.size() > L * 1024u) throw err::not_extended(es << "Body size can't be biger than : " << L << "KB");
      if (boundary[0xc] == 'j') {//application/json
        // json j = json::parse(value);
        throw err::not_extended(value.data());
      }
      if (boundary[0xc] == 'x') {//x-www-form-urlencoded; charset=UTF-8
        throw err::not_extended(value.data());
      }
      if (boundary[0] == 't') {//text/plain;charset=UTF-8
        try {
          //json j = json::parse(value);
          //throw std::runtime_error(j.dump());
          throw err::not_extended(value.data());
        } catch (const std::exception& e) {
          throw std::runtime_error(e.what());
        }
      }
      if (value.size() < 45u) throw err::bad_request("Wrong size!");
    }
    std::string_view content = content_length_ ? const_cast<Req&>(req).cache_file->getStringView() : std::string_view(value.c_str(), value.length());
    std::string_view::size_type f = content.find(boundary);
    content = content.substr(f + boundary.size() + 2, content.size()); std::string_view s; _:;
    if (content.size() > 2) {
      f = content.find(boundary);
      s = content.substr(0, f - 0xf);
      params.emplace_back(p_s(s));
      content = content.substr(f + boundary.size() + 2, content.size()); goto _;
    }
    if (params.size() == 0) throw err::not_found("Not Found!"); co_return;
  }
  param BP::p_s(std::string_view & s) {
    class param p;
    size_t f = s.find("\r\n\r\n");
    std::string lines(s.substr(0, f + 2));
    s = s.substr(f + 4);
    f = lines.find(';');
    if (f != std::string::npos) lines.erase(0, f + 2);
    f = lines.find("\r\n");
    std::string line = lines.substr(0, f);
    lines.erase(0, f + 2);
    char b = 0;
    while (!line.empty()) {
      const char* c = line.c_str() + 6; f = 0x9; while (*++c != '"' && ++f);
      std::string value = line.substr(0, f);
      if (b == '\0') {
        if (*++c == ';') { f += 2; value = value.substr(0, f - 3); }
        line.erase(0, f + 2); f = value.find('=');
        value = value.substr(f + 2); value.pop_back();
        p.key = value; ++b;
      } else if (b == '\1') {
        if (f != std::string::npos) line.erase(0, f + 2); else line.clear();
        f = value.find('=');
        value = value.substr(f + 2); value.pop_back();
        std::string::iterator i = value.end() - 1; if (*--i == '.')goto _; if (*--i == '.')goto _;
        if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
        if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
        throw err::bad_request("Suffix does not exist or exceeds 8 digits!");
      _:std::string s = menu + value;
        p.value = s; ++b;
      }
    }
    p.data = s.substr(0, s.size() - 2);
    if (_unlikely(b == '\2')) {
      f = lines.find("\r\n");
      line = lines.substr(0, f);
      lines.erase(0, f + 2);
      f = line.rfind(';');
      std::string h = line.substr(0, f);
      if (f != std::string::npos) line.erase(0, f + 2); else line.clear();
      //f = h.find(':');
      //p.type = h.substr(f + 2);
      p.size = p.data.size();
      h = fc::directory_ + p.value;
      struct stat64 ps;
      int ret = stat64(h.c_str(), &ps);
      if (!ret && ps.st_mode & S_IFREG) {
        if (ps.st_size == p.size) return p;
        std::ofstream of(h, std::ios::trunc | std::ios::out | std::ios::binary);
        if (!of.is_open() || of.fail()) {
          throw std::runtime_error("Failed to open1;");
        }
        of << p.data;
        if (of.fail()) {
          throw std::runtime_error("Failed to write1;");
        }
        of.close(); return p;
      };
      std::ofstream of(h, std::ios::out | std::ios::app | std::ios::binary);
      if (!of.is_open() || of.fail()) {
        throw std::runtime_error("Failed to open2;");
      }
      of << p.data;
      if (of.fail()) {
        throw std::runtime_error("Failed to write2;");
      }
      of.close();
    } else { p.value += p.data; }
    return p;
  }
}
