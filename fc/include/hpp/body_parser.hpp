#pragma once
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
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <set>
#include <h/common.h>
#include <req-res.hh>
#include <directory.hh>
#include <http_error.hh>
#ifdef WIN32
#include <psapi.h>
#include <direct.h>
#include <process.h>
#else
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <unistd.h>
#endif
namespace fc {
#ifdef WIN32
  float GetMemUsage(int pid = _getpid()) {
    uint64_t m = 0, n = 0; PROCESS_MEMORY_COUNTERS pmc; HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc))) m = pmc.WorkingSetSize, n = pmc.PagefileUsage; CloseHandle(h); return m / 1048576.0;
#else
  float GetMemUsage(int pid = getpid()) {
    char file_name[64] = { 0 }, line_buff[512] = { 0 }; sprintf(file_name, "/proc/%d/status", pid); FILE* fd = fopen(file_name, "r");
    if (nullptr == fd) return 0; int v = 0; for (int i = 0; i < 22; ++i) char* cha = fgets(line_buff, sizeof(line_buff), fd);
    sscanf(line_buff, "%s %d", file_name, &v); fclose(fd); return v / 1024.0;
#endif
  }
  using namespace std;
  static std::set<const char*> RES_menu = {};
  struct param { size_t size = 0; string key; string value; string filename; /*string type;*/ };
  ///The parsed multipart Req/Res (Length[ MB for file, KB for without file ]),(Bool is_file)//MD5?
  struct BP {
    const Req& req; string boundary, menu; vector<param> params; unsigned short L; bool ban_file;
    unsigned int content_length_;//string content_type = "multipart/form-data";
    BP(Req& req, const char* m, unsigned short mb = 32): menu(fc::upload_path_), L(mb), req(req), ban_file(false),
      boundary(g_b(fc::get_header(req.headers, RES_CT))), content_length_(req.length) {
      menu += m; if (RES_menu.find(m) == RES_menu.end()) {
        if (menu[menu.size() - 1] != '/')menu.push_back('/'); std::string ss(fc::directory_); ss += menu;
        RES_menu.insert(m); if (!fc::is_directory(ss)) { fc::create_directory(ss); }
      }
      assert(L < RES_USE_MAX_MEM_SIZE_MB); p_b(req.body);
    }
    BP(Req& req, unsigned short mb = 32, bool b = false): menu(fc::upload_path_), L(mb), req(req), ban_file(b),
      boundary(g_b(fc::get_header(req.headers, RES_CT))), content_length_(req.length) {
      assert(L < RES_USE_MAX_MEM_SIZE_MB); p_b(req.body);
    }
  private: //get_boundary
    string g_b(const Buf& h) const {
      //std::cout << "<" << h << ">" << h.size() << std::endl;
      unsigned int f = h.find("=----"); if (f != -1) return h.substr(f + 0xe).b2s(); return h.b2s();//raw
    }
    //parse_body
    void p_b(Buf& value) {//std::cout<<boundary<<std::endl;
      if (content_length_) {
        if (ban_file) throw err::not_implemented(Buf(30) << std::string_view("File not allowed!", 17));
        if (content_length_ > L * 1048576u) throw err::bad_request(Buf(40) << "Body size can't be biger than : " << L << "MB");
        float mem{ GetMemUsage() }; value.resize(content_length_);//not finish read
        if (mem > RES_USE_MAX_MEM_SIZE_MB) throw err::internal_server_error(Buf() << "insufficient memory!" << mem);
#ifdef _WIN32
        int count = req.fiber.read(value.data_, content_length_); unsigned int offset = 0;
        while (content_length_ > offset) {
          if (count > 0) {
            offset += count; count = req.fiber.read(value.data_ + offset, content_length_);
          } else count = req.fiber.read(value.data_ + offset, content_length_);
        }
#else
        int count = req.fiber.read(value.data_, content_length_); unsigned int offset = count > 0 ? count : 0;
        while (content_length_ > offset) {
          if (count > 0) {
            count = req.fiber.read(value.data_ + offset, content_length_); offset += count;
          } else count = req.fiber.read(value.data_ + offset, content_length_);
        }
#endif // _WIN32
        value.end_ += content_length_;
      } else {
        if (value.size() < 45u) throw err::bad_request("Wrong size!");
        if (value.size() > L * 1024u) throw err::not_extended(Buf(40) << "Body size can't be biger than : " << L << "KB");
        if (boundary[0xc] == 'j') {//application/json
          // json j = json::parse(value);
          throw err::not_extended(value);
        }
        if (boundary[0xc] == 'x') {//x-www-form-urlencoded; charset=UTF-8
          throw err::not_extended(value);
          throw err::not_extended("Wrong application/x-www-form-urlencoded!");
        }
        if (boundary[0] == 't') {//text/plain;charset=UTF-8
          try {
            //json j = json::parse(value);
            //throw std::runtime_error(j.dump());
          } catch (const std::exception& e) {
            throw std::runtime_error(e.what());
          }
        }
      }
      unsigned int f = value.find(boundary);
      value.erase(0, f + (unsigned int)boundary.length() + 2); std::string s(64, 0); _:;
      if (value.size() > 2) {
        f = value.find(boundary);
        s = value.substr(0, f - 0xf).b2s();
        params.emplace_back(p_s(s));
        value.erase(0, f + (unsigned int)boundary.length() + 2); goto _;
      }
      if (params.size() == 0) throw err::not_found("Not Found!");
    }
    //parse_section
    param p_s(std::string& s) {
      struct param p;
      size_t f = s.find("\r\n\r\n");
      string lines(s.substr(0, f + 2));
      s.erase(0, f + 4);
      f = lines.find(';');
      if (f != string::npos) lines.erase(0, f + 2);
      f = lines.find("\r\n");
      string line = lines.substr(0, f);
      lines.erase(0, f + 2);
      char b = 0;
      while (!line.empty()) {
        const char* c = line.c_str() + 7; f = 9; while (*++c != '"' && ++f);
        string value = line.substr(0, f);
        if (b == '\0') {
          if (*++c == ';') { f += 2; }
          line.erase(0, f + 2); f = value.find('=');
          value = value.substr(f + 2); value.pop_back();
          p.key = DecodeURL(value); ++b;
        } else if (b == '\1') {
          if (f != string::npos) line.erase(0, f + 2); else line.clear();
          f = value.find('=');
          value = value.substr(f + 2); value.pop_back();
          string::iterator i = --value.end(); if (*--i == '.')goto _; if (*--i == '.')goto _;
          if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
          if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
          throw err::bad_request("Suffix does not exist or exceeds 8 digits!");
        _:std::string s = menu + value;
          p.filename = DecodeURL(s); ++b;
        }
      }
      p.value = s.substr(0, s.length() - 2);
      if (b == '\2') {
        f = lines.find("\r\n");
        line = lines.substr(0, f);
        lines.erase(0, f + 2);
        f = line.rfind(';');
        string h = line.substr(0, f);
        if (f != string::npos) line.erase(0, f + 2); else line.clear();
        //f = h.find(':');
        //p.type = h.substr(f + 2);
        p.size = p.value.length();
        h = fc::directory_ + p.filename;
        struct stat ps;
        int ret = stat(h.c_str(), &ps);
        if (!ret && ps.st_mode & S_IFREG) {
          if (ps.st_size == p.size) return p;
          std::ofstream of(h, ios::trunc | ios::out | ios::binary);
          of << p.value; of.close(); return p;
        };
        std::ofstream of(h, ios::out | ios::app | ios::binary);
        of << p.value; of.close();
      }
      return p;
    }
  };
}
