#ifndef BODY_PARSER_HPP
#define BODY_PARSER_HPP
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
#include <vector>
#include <set>
#include <hpp/string_view.hpp>
#include <req-res.hh>
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
  float GetMemUsage(int pid = _getpid());
#else
  float GetMemUsage(int pid = getpid());
#endif
  class param;
  static std::set<std::string> RES_menu = {};
  ///The parsed multipart Req/Res (Length[ MB for file, KB for without file ]),(Bool is_file)// MAX_SIZE < 4GB
  struct BP {
    const Req& req; std::string_view boundary; std::string menu; std::vector<param> params; unsigned short L; bool ban_file;
    //string content_type = "multipart/form-data";
#ifdef _WIN32
    long long content_length_;
#else
    long content_length_;
#endif // _WIN32
    BP(Req& req, const char* m, unsigned short mb = 32);
    BP(Req& req, unsigned short mb = 32, bool b = false);
    ~BP() { if (content_length_) { req.cache_file.reset(); } }
  private:
    //get_boundary
    std::string_view g_b(const std::string_view& h) const;
    //parse_body
    void p_b(std::string_view& value);
    //parse_section
    param p_s(std::string_view& s);
  };
  class param { std::string_view data; public:std::string key; std::string value; size_t size = 0; /*string type;*/friend BP; };
}
#endif
