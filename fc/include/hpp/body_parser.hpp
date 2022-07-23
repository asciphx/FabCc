#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <h/common.h>
#include <req-res.hh>
#include <directory.hh>
//from https://github.com/asciphx/Crow/blob/master/include/cc/body_parser.h
namespace fc {
  using namespace std;
  static std::set<const char*> RES_menu = {};
  struct param { size_t size = 0; string key; string value; string filename; /*string type;*/ };
  ///The parsed multipart Req/Res (Length[ kb ]),(Bool is_file)//MD5?
  template<unsigned short L, bool B = false>
  struct BP {
	const str_map* headers; string_view boundary; string menu; vector<param> params; //string content_type = "multipart/form-data";
	~BP() { headers = nullptr; }
	BP(Req& req, const char* m): headers(&(req.headers)), menu(detail::upload_path_),
	  boundary(g_b(fc::get_header(*headers, RES_CT))) {
	  menu += m; if (RES_menu.find(m) == RES_menu.end()) {
		if (menu[menu.size() - 1] != '/')menu.push_back('/'); std::string ss(detail::directory_); ss += menu;
		RES_menu.insert(m); if (!fc::is_directory(ss)) { fc::create_directory(ss); }
	  }
	  p_b(req.body);
	}
	BP(Req& req): headers(&(req.headers)), menu(detail::upload_path_),
	  boundary(g_b(fc::get_header(*headers, RES_CT))) {
	  p_b(req.body);
	}
  private: //get_boundary
	string g_b(const string& h) const {
	  //std::cout << "<" << h << ">" << h.size() << std::endl;
	  size_t f = h.find("=----"); if (f != std::string::npos) return h.substr(f + 0xe); return h;//raw
	}
	//parse_body
	void p_b(string value) {//std::cout<<boundary<<std::endl;
	  if (boundary[0xc] == 'j') {//application/json
		// json j = json::parse(value);
		throw std::runtime_error(value);
	  } else if (boundary[0xc] == 'x') {//x-www-form-urlencoded; charset=UTF-8
		throw std::runtime_error(value);
		throw std::runtime_error("Wrong application/x-www-form-urlencoded!");
	  } else if (boundary[0] == 't') {//text/plain;charset=UTF-8
		try {
		  //json j = json::parse(value);
		  //throw std::runtime_error(j.dump());
		} catch (const std::exception& e) {
		  throw std::runtime_error(e.what());
		}
	  }
	  if (value.size() < 45) throw std::runtime_error("Wrong value size!");
	  if (value.size() > L * 1024) throw std::runtime_error(std::string("Body size can't be biger than : ") + std::to_string(L) + "kb");
	  size_t f = value.find(boundary);
	  value.erase(0, f + boundary.length() + 2); string s; _:;
	  if (value.size() > 2) {
		f = value.find(boundary);
		s = value.substr(0, f - 0xf);
		params.emplace_back(p_s(s));
		value.erase(0, f + boundary.length() + 2); goto _;
	  }
	  if (params.size() == 0) throw std::runtime_error("Not Found!");
	}
	//parse_section
	param p_s(string& s) {
	  struct param p;
	  size_t f = s.find("\r\n\r\n");
	  string lines = s.substr(0, f + 2);
	  s.erase(0, f + 4);
	  f = lines.find(';');
	  if (f != string::npos) lines.erase(0, f + 2);
	  f = lines.find("\r\n");
	  string line = lines.substr(0, f);
	  lines.erase(0, f + 2);
	  char b = 0;
	  while (!line.empty()) {
		f = line.find(';');
		string value = line.substr(0, f);
		if (f != string::npos) line.erase(0, f + 2); else line.clear();
		f = value.find('=');
		value = value.substr(f + 2); value.pop_back();
		if (b == '\0') {
		  p.key = value; ++b;
		} else if (b == '\1') {
		  string::iterator i = --value.end(); if (*--i == '.')goto _; if (*--i == '.')goto _;
		  if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
		  if (*--i == '.')goto _; if (*--i == '.')goto _; if (*--i == '.')goto _;
		  throw std::runtime_error("Suffix does not exist or exceeds 8 digits!");
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
		h = detail::directory_ + p.filename;
		struct stat ps;
		int ret = stat(h.c_str(), &ps);
		if (!ret) {
		  if (ps.st_mode & S_IFREG) {
			if (ps.st_size == p.size) return p;
			std::ofstream of(h, ios::trunc | ios::out | ios::binary);
			of << p.value; of.close(); return p;
		  }
		  //if (ps.st_mode & S_IFDIR) throw std::runtime_error("Folder already exists!");//87 line
		};
		std::ofstream of(h, ios::out | ios::app | ios::binary);
		of << p.value; of.close();
	  }
	  return p;
	}
  };
}
