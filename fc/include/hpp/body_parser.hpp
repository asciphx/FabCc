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
namespace fc {
  using namespace std;
  static std::set<const char*> RES_menu = {};
  struct param { size_t size = 0; string key; string value; string filename; /*string type;*/ };
  ///The parsed multipart Req/Res (Length[ MB ]),(Bool is_file)//MD5?
  struct BP {
    const str_map* headers; string boundary, menu; vector<param> params; unsigned short L;//string content_type = "multipart/form-data";
    ~BP() { headers = nullptr; }
    BP(Req& req, const char* m, unsigned short mb = 32): headers(&(req.headers)), menu(fc::upload_path_), L(mb),
      boundary(g_b(fc::get_header(*headers, RES_CT))) {
      menu += m; if (RES_menu.find(m) == RES_menu.end()) {
        if (menu[menu.size() - 1] != '/')menu.push_back('/'); std::string ss(fc::directory_); ss += menu;
        RES_menu.insert(m); if (!fc::is_directory(ss)) { fc::create_directory(ss); }
      }
      p_b(req.body);
    }
    BP(Req& req, unsigned short mb = 32): headers(&(req.headers)), menu(fc::upload_path_), L(mb),
      boundary(g_b(fc::get_header(*headers, RES_CT))) {
      p_b(req.body);
    }
  private: //get_boundary
    string g_b(const Buf& h) const {
      //std::cout << "<" << h << ">" << h.size() << std::endl;
      unsigned int f = h.find("=----"); if (f != -1) return h.substr(f + 0xe).b2s(); return h.b2s();//raw
    }
    //parse_body
    void p_b(Buf& value) {//std::cout<<boundary<<std::endl;
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
      if (value.size() < 45) throw err::not_implemented("Wrong value size!");
      if (value.size() > L * 1048576u) throw err::not_extended(std::string("Body size can't be biger than : ") + std::to_string(L) + "MB");
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
