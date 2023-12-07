#include <hh/directory.hh>
namespace fc {
  bool crossplatform_realpath(const std::string& path, char out_buffer[CROSSPLATFORM_MAX_PATH]) {
    // Check if file exists by real file path.
#if defined _WIN32
    return 0 != GetFullPathNameA(path.c_str(), CROSSPLATFORM_MAX_PATH, out_buffer, nullptr);
#else
    return nullptr != realpath(path.c_str(), out_buffer);
#endif
  }
  int create_directory(const std::string& path) {
#if defined _WIN32
#if _MSC_VER < 1700
    int ret = mkdir(path.c_str());
#else
    int ret = _mkdir(path.c_str());
#endif
#else
    int ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    if (ret && errno == EEXIST) {
      return 1;//DEBUG("\"%s\" aleardy exist\n", path.c_str());
    } else if (ret) {
      printf("create error: %d, :%s\n", ret, strerror(errno)); return -1;
    }
    return 0;
  }
  bool is_regular_file(const std::string& path) {
    struct stat64 ps; if (-1 == stat64(path.c_str(), &ps)) return false; return ps.st_mode & S_IFREG;
  }
  bool is_regular_file(const char* path) {
    struct stat64 ps; if (-1 == stat64(path, &ps)) return false; return ps.st_mode & S_IFREG;
  }
  bool is_directory(const std::string& path) {
    struct stat64 ps; if (-1 == stat64(path.c_str(), &ps)) return false; return ps.st_mode & S_IFDIR;
  }
  bool is_directory(const char* path) {
    struct stat64 ps; if (-1 == stat64(path, &ps)) return false; return ps.st_mode & S_IFDIR;
  }
  bool exists(const char* path) {
#ifdef _WIN32
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat64 attr; return ::lstat64(path, &attr) == 0;
#endif
  }
  bool starts_with(const char* pre, const char* str) {
    size_t lp = strlen(pre), ls = strlen(str); return ls < lp ? false : memcmp(pre, str, lp) == 0;
  }
  std::vector<fc::dir_iter> directory_iterator(const std::string& p, bool sub, int n) {
    std::vector<fc::dir_iter> v; if (n == 0) return v; bool b; fc::dir_iter di;
#ifdef _WIN32
    size_t $ = p.size() - 1; _finddatai64_t _; long long l;
    if (p[$] != '\\' && p[$] != '/') const_cast<std::string&>(p).append({ '\\' }); std::string s = p + '*';
    if ((l = _findfirsti64(s.c_str(), &_)) != -1) {
      do {
        b = _.name[0] == '.';
        if (!((b && _.name[1] == 0) || (b && _.name[1] == '.' && _.name[2] == 0))) {
          di.size = _.size;
#ifdef __MINGW32__
          int path_len = MultiByteToWideChar(CP_ACP, 0, _.name, -1, NULL, 0);
          wchar_t* pwsz = new wchar_t[path_len]; MultiByteToWideChar(CP_ACP, 0, _.name, -1, pwsz, path_len);
          std::wstring wstr(pwsz, path_len); delete[] pwsz; pwsz = nullptr; const wchar_t* unicode = wstr.c_str();
          path_len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
          char* szUtf8 = (char*)malloc(path_len + 1); szUtf8[path_len] = 0;
          WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, path_len, NULL, NULL);
          s = p + szUtf8; free(szUtf8);//MINGW does not support some languages unless they exist in the encoding list
#else
          s = p + _.name;
#endif
          if (sub && (_.attrib & _A_SUBDIR)) {
            s.push_back('\\'); di.name = std::move(s); v.push_back(di);
            std::vector<fc::dir_iter> r = directory_iterator(di.name, sub, --n); v.insert(v.end(), r.begin(), r.end()); continue;
          }
          if (_.attrib & _A_ARCH) {
            di.name = s; size_t l = s.rfind('.'); if (l != -1 && s.size() - l < 8) {
              s = std::move(s.substr(l + 1, s.size())); memcpy(di.ext, s.c_str(), s.size()); if(s.size()!=8)di.ext[s.size()] = 0;
            } v.push_back(di);
          }
        }
      } while (_findnexti64(l, &_) == 0);
      _findclose(l);
    }
#else
    if (p[p.size() - 1] != '/') const_cast<std::string&>(p).append({ '/' });
    DIR* dir = opendir(p.data()); dirent* d; std::string s; struct stat64 ps;
    while ((d = readdir(dir)) != NULL) {
      b = d->d_name[0] == '.';
      if (!((b && d->d_name[1] == 0) || (b && d->d_name[1] == '.' && d->d_name[2] == 0))) {
        s = p + d->d_name;
        if (sub && (d->d_type & DT_DIR)) {
          s.push_back('/'); di.name = std::move(s); stat64(di.name.c_str(), &ps); di.size = ps.st_size; v.push_back(di);
          std::vector<fc::dir_iter> r = directory_iterator(di.name, sub, --n); v.insert(v.end(), r.begin(), r.end()); continue;
        }
        if (d->d_type & DT_REG) {
          di.name = s; stat64(s.c_str(), &ps); di.size = ps.st_size; size_t l = s.rfind('.'); if (l != -1 && s.size() - l < 8) {
            s = std::move(s.substr(l + 1, s.size())); memcpy(di.ext, s.c_str(), s.size()); if(s.size()!=8)di.ext[s.size()] = 0;
          }
          v.push_back(di);
        }
      }
    }
    closedir(dir);
#endif
    return v;
  }
} // namespace fc
