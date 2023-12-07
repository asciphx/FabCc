#include "hh/edge.hh"
#include "hh/lexical_cast.hh"
DEF_string(help, "", ">>.help info");
DEF_string(config, "", ">>.path of config file", conf);
DEF_string(version, "", ">>.version of the program");
DEF_bool(mkconf, false, ">>.generate config file");
namespace edge {
  namespace xx {
    struct Edge {
      Edge(char type, const char* name, const char* alias, const char* value, const char* help,
        const char* file, int line, void* addr);
      std::string set_value(const std::string& v);
      std::string get_value() const;
      void print() const;
      const char* type_str() const;
      char type;
      bool inco;          // flag inside co (comment starts with >>)
      const char* name;
      const char* alias;  // alias for this flag
      const char* value;  // default value
      const char* help;   // help info
      const char* file;   // file where the flag is defined
      int line;           // line of the file where the flag is defined
      int lv;             // level: 0-9
      void* addr;         // point to the flag variable
    };
    inline std::map<std::string, Edge*>& gFlags() {
      static auto flags = std::make<std::map<std::string, Edge*>>();
      return *flags;
    }
    const char TYPE_string = 's';
    const char TYPE_bool = 'b';
    const char TYPE_int32 = 'i';
    const char TYPE_int64 = 'I';
    const char TYPE_uint32 = 'u';
    const char TYPE_uint64 = 'U';
    const char TYPE_double = 'd';
    Edge::Edge(char type, const char* name, const char* alias, const char* value,
      const char* help, const char* file, int line, void* addr)
      : type(type), inco(false), name(name), alias(alias), value(value),
      help(help), file(file), line(line), lv(5), addr(addr) {
      // flag defined in co
      if (help[0] == '>' && help[1] == '>') {
        this->inco = true;
        this->help += 2;
      }
      // get level(0-9) at the beginning of help
      const char* const h = this->help;
      if (h[0] == '#' && '0' <= h[1] && h[1] <= '9' && (h[2] == ' ' || h[2] == '\0')) {
        lv = h[1] - '0';
        this->help += 2 + !!h[2];
      }
    }
    std::string Edge::set_value(const std::string& v) {
      switch (this->type) {
      case TYPE_string:
        *static_cast<std::string*>(this->addr) = v;
        return std::string();
      case TYPE_bool:
        *static_cast<bool*>(this->addr) = std::lexical_cast<bool>(const_cast<std::string&>(v));
        break;
      case TYPE_int32:
        *static_cast<i32*>(this->addr) = std::lexical_cast<i32>(const_cast<std::string&>(v));
        break;
      case TYPE_uint32:
        *static_cast<u32*>(this->addr) = std::lexical_cast<u32>(const_cast<std::string&>(v));
        break;
      case TYPE_int64:
        *static_cast<i64*>(this->addr) = std::lexical_cast<i64>(const_cast<std::string&>(v));
        break;
      case TYPE_uint64:
        *static_cast<u64*>(this->addr) = std::lexical_cast<u64>(const_cast<std::string&>(v));
        break;
      case TYPE_double:
        *static_cast<double*>(this->addr) = std::lexical_cast<double>(const_cast<std::string&>(v));
        break;
      default:
        return "unknown flag type";
      } return std::string();
      // switch (std::error()) {
      // case 0:
       //return std::string();
      // case ERANGE:
       //return "out of range";
      // default:
       //return "invalid value";
      // }
    }
    template<typename T>
    std::string int_to_string(T t) {
      if ((0 <= t && t <= 8192) || (t < 0 && t >= -8192)) return std::lexical_cast<std::string>(t);
      const char* u = "kmgtp";
      int i = -1;
      while (t != 0 && (t & 1023) == 0) {
        t >>= 10;
        if (++i >= 4) break;
      }
      std::string s = std::lexical_cast<std::string>(t);
      if (i >= 0) s.append({ u[i] });
      return s;
    }
    std::string Edge::get_value() const {
      switch (this->type) {
      case TYPE_string:
        return *static_cast<std::string*>(this->addr);
      case TYPE_bool:
        return std::lexical_cast<std::string>(*static_cast<bool*>(this->addr));
      case TYPE_int32:
        return int_to_string(*static_cast<i32*>(this->addr));
      case TYPE_uint32:
        return int_to_string(*static_cast<u32*>(this->addr));
      case TYPE_int64:
        return int_to_string(*static_cast<i64*>(this->addr));
      case TYPE_uint64:
        return int_to_string(*static_cast<u64*>(this->addr));
      case TYPE_double:
        return std::lexical_cast<std::string>(*static_cast<double*>(this->addr));
      default:
        return "unknown flag type";
      }
    }
    inline const char* Edge::type_str() const {
      switch (this->type) {
      case TYPE_string: return "string";
      case TYPE_bool:   return "bool";
      case TYPE_int32:  return "i32";
      case TYPE_uint32: return "u32";
      case TYPE_int64:  return "i64";
      case TYPE_uint64: return "u64";
      case TYPE_double: return "double";
      default:          return "unknown flag type";
      }
    }
    inline void Edge::print() const {
      std::cout << color::green << "    -" << this->name;
      if (*this->alias) std::cout << ", " << this->alias;
      std::cout.flush();
      std::cout << color::blue << "  " << this->help << '\n' << color::deflt
        << "\ttype: " << this->type_str()
        << "\t  default: " << this->value
        << "\n\tfrom: " << this->file
        << std::endl;
    }
    void add_flag(
      char type, const char* name, const char* value, const char* help,
      const char* file, int line, void* addr, const char* alias) {
      auto f = std::make<Edge>(type, name, alias, value, help, file, line, addr);
      auto r = gFlags().insert(std::make_pair(name, f));
      if (!r.second) {
        std::cout << "multiple definitions of flag: " << name << ", from "
          << r.first->second->file << " and " << file << std::endl;
        exit(0);
      }
      if (alias[0]) {
        auto v = std::split(alias, ',');
        for (auto& x : v) {
          std::strip(x);
          auto r = gFlags().insert(std::make_pair(x, f));
          if (!r.second) {
            std::cout << "alias " << name << " as " << x << " failed, flag " << x
              << " already exists in " << r.first->second->file << std::endl;
            exit(0);
          }
        }
      }
    }
    inline Edge* find_flag(const std::string& name) {
      auto it = gFlags().find(name);
      return it != gFlags().end() ? it->second : NULL;
    }
    // Return error message on any error.
    std::string set_flag_value(const std::string& name, const std::string& value) {
      Edge* flag = find_flag(name);
      if (!flag) return "flag not defined: " + name;
      std::string err = flag->set_value(value);
      if (!err.empty()) err.append(": ").append(value);
      return err;
    }
    // set_bool_flags("abc"):  -abc -> true  or  -a, -b, -c -> true
    std::string set_bool_flags(const std::string& name) {
      Edge* flag = find_flag(name);
      if (flag) {
        if (flag->type == TYPE_bool) {
          *static_cast<bool*>(flag->addr) = true;
          return std::string();
        } else {
          return std::string("value not set for non-bool flag: ").append(name);
        }
      }
      if (name.size() == 1) {
        return std::string("undefined bool flag: ").append(name);
      }
      for (size_t i = 0; i < name.size(); ++i) {
        flag = find_flag(name.substr(i, 1));
        if (!flag) {
          return std::string("undefined bool flag -").append({ name[i] }).append(" in -", 5) << std::string_view(name.data(), name.size());
        } else if (flag->type != TYPE_bool) {
          return std::string("-").append({ name[i] }).append(" is not bool in -", 17) << std::string_view(name.data(), name.size());
        } else {
          *static_cast<bool*>(flag->addr) = true;
        }
      }
      return std::string();
    }
    void show_user_flags() {
      bool the_first_one = true;
      for (auto it = gFlags().begin(); it != gFlags().end(); ++it) {
        const auto& f = *it->second;
        if (!f.inco && *f.help && (!*f.alias || it->first == f.name)) {
          if (the_first_one) {
            the_first_one = false;
            std::cout << "flags:\n";
          }
          f.print();
        }
      }
    }
    void show_co_flags() {
      std::cout << "flags:\n";
      for (auto it = gFlags().begin(); it != gFlags().end(); ++it) {
        const auto& f = *it->second;
        if (f.inco && *f.help && (!*f.alias || it->first == f.name)) {
          f.print();
        }
      }
    }
    void show_all_flags() {
      std::cout << "flags:\n";
      for (auto it = gFlags().begin(); it != gFlags().end(); ++it) {
        const auto& f = *it->second;
        if (*f.help && (!*f.alias || it->first == f.name)) {
          f.print();
        }
      }
    }
    // print FLG_help first if it is not empty, then print usage and user flags
    inline void show_help() {
      if (!FLG_help.empty()) {
        std::cout << FLG_help << std::endl; return;
      }
      std::cout << "usage:  " << color::blue << "$exe [-flag] [value]\n" << color::deflt
        << "\t" << "$exe -x -i 8k -s ok        # x=true, i=8192, s=\"ok\"\n"
        << "\t" << "$exe --                    # print all flags\n"
        << "\t" << "$exe -mkconf               # generate config file\n"
        << "\t" << "$exe -conf xx.conf         # run with config file\n\n";
      show_user_flags();
    }
    inline void show_version() {
      if (!FLG_version.empty()) std::cout << FLG_version << std::endl;
    }
    std::string format_str(const std::string& s) {
      size_t px = s.find('"');
      size_t py = s.find('\'');
      size_t pz = s.find('`');
      if (px == -1 && py == -1 && pz == -1) return s;
      if (px == -1) return std::string().append({ '"' }).append(s).append({ '"' });
      if (py == -1) return std::string().append({ '\'' }).append(s).append({ '\'' });
      if (pz == -1) return std::string().append({ '`' }).append(s).append({ '`' });
      return std::string().append("```").append(s).append("```");
    }
#define COMMENT_LINE_LEN 72
    void mkconf(const std::string& exe) {
      // Order flags by lv, file, line.  <lv, <file, <line, flag>>>
      std::map<int, std::map<std::string, std::map<int, Edge*>>> flags;
      for (auto it = gFlags().begin(); it != gFlags().end(); ++it) {
        Edge* f = it->second;
        if (f->help[0] == '.' || f->help[0] == '\0') continue; // ignore hidden flags.
        flags[f->lv][std::string(f->file)][f->line] = f;
      }
      std::string fname(exe);
      if (std::ends_with(fname, ".exe")) fname.resize(fname.size() - 4);
      fname += ".conf";
      std::ofstream f(fname.c_str(), std::ios::trunc | std::ios::out | std::ios::binary);
      if (!f) {
        std::cout << "can't open config file: " << fname << std::endl;
        return;
      }
      f << std::string(COMMENT_LINE_LEN, '#') << '\n'
        << "###  > # or // for comments\n"
        << "###  > k,m,g,t,p (case insensitive, 1k for 1024, etc.)\n"
        << std::string(COMMENT_LINE_LEN, '#') << "\n\n\n";
      for (auto it = flags.begin(); it != flags.end(); ++it) {
        const auto& x = it->second;
        for (auto xit = x.begin(); xit != x.end(); ++xit) {
          const auto& y = xit->second;
          f << "# >> " << std::replace(xit->first, "\\", "/") << '\n';
          f << "#" << std::string(COMMENT_LINE_LEN - 1, '=') << '\n';
          for (auto yit = y.begin(); yit != y.end(); ++yit) {
            const Edge& flag = *yit->second;
            std::string v = flag.get_value();
            if (flag.type == TYPE_string) v = format_str(v);
            f << "# " << std::replace(flag.help, "\n", "\n# ") << '\n';
            f << flag.name << " = " << v << "\n\n";
          }
          f << "\n";
        }
      }
      f.flush();
    }
#undef COMMENT_LINE_LEN
  // @kv:     for -a=b, or -a b, or a=b
  // @bools:  for -a, -xyz
  // return non-flag elements (etc. hello, -8, -8k, -, --, --- ...)
    std::vector<std::string> analyze(
      const std::vector<std::string>& args, std::map<std::string, std::string>& kv, std::vector<std::string>& bools
    ) {
      std::vector<std::string> res;
      for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        size_t bp = arg.find_first_not_of('-');
        size_t ep = arg.find('=');
        // @arg has only '-':  for -, --, --- ...
        if (bp == -1) {
          res.push_back(arg);
          continue;
        }
        if (ep <= bp) {
          std::cout << "invalid parameter" << ": " << arg << std::endl;
          exit(0);
        }
        // @arg has '=', for -a=b or a=b
        if (ep != -1) {
          kv[arg.substr(bp, ep - bp)] = arg.substr(ep + 1);
          continue;
        }
        // non-flag: etc. hello, -8, -8k ...
        if (bp == 0 || (bp == 1 && '0' <= arg[1] && arg[1] <= '9')) {
          res.push_back(arg);
          continue;
        }
        // flag: -a, -a b, or -j4
        {
          Edge* flag = 0;
          std::string next;
          std::string name = arg.substr(bp);
          // for -j4
          if (name.size() > 1 && (('0' <= name[1] && name[1] <= '9') || name[1] == '-')) {
            if (!find_flag(name) && find_flag(name.substr(0, 1))) {
              kv[name.substr(0, 1)] = name.substr(1);
              continue;
            }
          }
          if (i + 1 == args.size()) goto no_value;
          next = args[i + 1];
          if (next.find('=') != -1) goto no_value;
          if (next.front() == '-' && next.find_first_not_of('-') != -1) {
            if (next[1] < '0' || next[1] > '9') goto no_value;
          }
          flag = find_flag(name);
          if (!flag) goto no_value;
          if (flag->type != TYPE_bool) goto has_value;
          if (next == "0" || next == "1" || next == "false" || next == "true") goto has_value;
        no_value:
          bools.push_back(name);
          continue;
        has_value:
          kv[name] = next;
          ++i;
          continue;
        };
      }
      return res;
    }
    void parse_config(const std::string& config);
    std::vector<std::string> parse_command_line_flags(int argc, const char** argv) {
      if (argc <= 1) return std::vector<std::string>();
      std::vector<std::string> args;
      for (int i = 1; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
      }
      if (args.size() == 1 && std::starts_with(args[0], "--")) {
        const std::string& arg = args[0];
        if (arg.size() == 2) { /* arg == "--" */
          show_all_flags();
          exit(0);
        }
        if (arg == "--help") {
          show_help();
          exit(0);
        }
        if (arg == "--version") {
          show_version();
          exit(0);
        }
        if (arg == "--cocoyaxi") {
          show_co_flags();
          exit(0);
        }
      }
      std::map<std::string, std::string> kv;
      std::vector<std::string> bools;
      std::vector<std::string> v = analyze(args, kv, bools);
      auto it = kv.find("config");
      if (it == kv.end()) it = kv.find("conf");
      if (it != kv.end()) {
        FLG_config = it->second;
      } else if (!v.empty()) {
        if (std::ends_with(v[0], ".conf") || std::ends_with(v[0], "config")) {
          if (fc::exists(v[0].c_str())) FLG_config = v[0];
        }
      }
      if (!FLG_config.empty()) parse_config(FLG_config);
      for (it = kv.begin(); it != kv.end(); ++it) {
        std::string err = set_flag_value(it->first, it->second);
        if (!err.empty()) {
          std::cout << err << std::endl;
          exit(0);
        }
      }
      for (size_t i = 0; i < bools.size(); ++i) {
        std::string err = set_bool_flags(bools[i]);
        if (!err.empty()) {
          std::cout << err << std::endl;
          exit(0);
        }
      }
      return v;
    }
    std::string remove_quotes_and_comments(const std::string& s) {
      if (s.empty()) return s;
      std::string r;
      size_t p, q, l;
      char c = s[0];
      if (c == '"' || c == '\'' || c == '`') {
        if (std::starts_with(s, "```")) {
          p = s.find("```", 3);
          l = 3;
        } else {
          p = s.find(c, 1);
          l = 1;
        }
        if (p == -1) goto no_quotes;
        p = s.find_first_not_of(" \t", p + l);
        if (p == -1) {
          r = std::strip(s, " \t", 'r');
        } else if (s[p] == '#' || s.substr(p, 2) == "//") {
          r = std::strip(s.substr(0, p), " \t", 'r');
        } else {
          goto no_quotes;
        }
        return r.substr(l, r.size() - l * 2);
      }
    no_quotes:
      p = s.find('#');
      q = s.find("//");
      if (p == -1 && q == -1) return s;
      return std::strip(s.substr(0, p < q ? p : q), " \t", 'r');
    }
    std::string getline(std::vector<std::string>& lines, size_t n) {
      std::string line;
      while (n < lines.size()) {
        std::string s(lines[n++]);
        std::replace(s, "　", " ");  // replace Chinese spaces
        std::strip(s);
        if (s.empty() || s.back() != '\\') {
          line += s;
          return line;
        }
        line += std::strip(s, " \t\r\n\\", 'r');
      }
      return line;
    }
    void parse_config(const std::string& config) {
      std::ifstream f(config.c_str(), std::ios::in | std::ios::binary);
      if (!f) {
        std::cout << "can't open config file: " << config << std::endl;
        exit(0);
      }
      char readBuf[8192]; std::string data;
      int l = 0; $:l = (int)f.read(readBuf, 8192).gcount();
      if (l) { data += std::string_view(readBuf, sizeof(readBuf)); goto $; }
      f.close();
      char sep = '\n';
      if (data.find('\n') == -1 && data.find('\r') != -1) sep = '\r';
      auto lines = std::split(data, sep);
      size_t lineno = 0; // line number of config file.
      for (size_t i = 0; i < lines.size();) {
        lineno = i;
        std::string s = getline(lines, i);
        if (s.empty() || s[0] == '#' || std::starts_with(s, "//")) continue;
        size_t p = s.find('=');
        if (p == 0 || p == -1) {
          std::cout << "invalid config: " << s << ", at " << config << ':' << (lineno + 1) << std::endl;
          exit(0);
        }
        std::string flg = std::strip(s.substr(0, p), " \t", 'r');
        std::string val = std::strip(s.substr(p + 1), " \t", 'l');
        val = remove_quotes_and_comments(val);
        std::string err = set_flag_value(flg, val);
        if (!err.empty()) {
          std::string er = err.substr(0, 16);
          if (!(er == "flag not defined")) {
            std::cout << err << ", at " << config << ':' << (lineno + 1) << std::endl;
            exit(0);
          } else {
            std::cout << "WARNING: " << err << ", at " << config << ':' << (lineno + 1) << std::endl;
          }
        }
      }
    }
  } // namespace xx
  std::vector<std::string> init(int argc, const char** argv) {
    std::vector<std::string> v = xx::parse_command_line_flags(argc, argv);
    if (FLG_mkconf) {
      xx::mkconf(argv[0]);
      exit(0);
    }
    return v;
  }
  void init(const std::string& path) {
    xx::parse_config(path);
  }
  std::string set_value(const std::string& name, const std::string& value) {
    return xx::set_flag_value(name, value);
  }
  bool alias(const char* name, const char* new_name) {
    auto flag = xx::find_flag(name);
    if (!flag || !*new_name) return false;
    auto r = xx::gFlags().insert(std::make_pair(std::string(new_name), flag));
    if (!r.second) return false; // new_name already exists
    flag->alias = new_name;
    return true;
  }
} // namespace flag
