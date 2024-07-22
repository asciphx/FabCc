#include "hh/query_string.hh"
namespace cc {
  // TODO: implement sorting of the qs_kv array; for now ensure it's not compiled
  // isxdigit _is_ available in <ctype.h>, but let's avoid another header instead
#define QS_ISHEX(x)                                        \
  ((((x) >= '0' && (x) <= '9') || ((x) >= 'A' && (x) <= 'F') || \
    ((x) >= 'a' && (x) <= 'f'))                                 \
       ? 1                                                      \
       : 0)
#define QS_HEX2DEC(x)               \
  (((x) >= '0' && (x) <= '9')   ? (x)-48 \
   : ((x) >= 'A' && (x) <= 'F') ? (x)-55 \
   : ((x) >= 'a' && (x) <= 'f') ? (x)-87 \
                                : 0)
#define QS_ISQSCHR(x) \
  ((((x) == '=') || ((x) == '#') || ((x) == '&') || ((x) == '\0')) ? 0 : 1)
  int qs_strncmp(const char* s, const char* qs, size_t n) {
    int i = 0;
    unsigned char u1, u2, unyb, lnyb;
    while (n-- > 0) {
      u1 = static_cast<unsigned char>(*s++);
      u2 = static_cast<unsigned char>(*qs++);
      if (!QS_ISQSCHR(u1)) {
        u1 = '\0';
      }
      if (!QS_ISQSCHR(u2)) {
        u2 = '\0';
      }
      if (u1 == '+') {
        u1 = ' ';
      }
      if (u1 == '%')  // easier/safer than scanf
      {
        unyb = static_cast<unsigned char>(*s++);
        lnyb = static_cast<unsigned char>(*s++);
        if (QS_ISHEX(unyb) && QS_ISHEX(lnyb))
          u1 = (QS_HEX2DEC(unyb) * 16) + QS_HEX2DEC(lnyb);
        else
          u1 = '\0';
      }
      if (u2 == '+') {
        u2 = ' ';
      }
      if (u2 == '%')  // easier/safer than scanf
      {
        unyb = static_cast<unsigned char>(*qs++);
        lnyb = static_cast<unsigned char>(*qs++);
        if (QS_ISHEX(unyb) && QS_ISHEX(lnyb))
          u2 = (QS_HEX2DEC(unyb) * 16) + QS_HEX2DEC(lnyb);
        else
          u2 = '\0';
      }
      if (u1 != u2) return u1 - u2;
      if (u1 == '\0') return 0;
      ++i;
    }
    if (QS_ISQSCHR(*qs))
      return -1;
    else
      return 0;
  }//int qs_parse(char* qs, char* qs_kv[], int qs_kv_size) {
  int qs_parse(char* substr_ptr, char* qs_kv[], int qs_kv_size) {
    int i, j;
    //char* substr_ptr;
    //for (i = 0; i < qs_kv_size; ++i) qs_kv[i] = NULL;
    //// find the beginning of the k/v substrings or the fragment
    //substr_ptr = qs + strcspn(qs, "?#");
    //if (substr_ptr[0] != '\0')
    //  ++substr_ptr;
    //else
    //  return 0;  // no query or fragment
    i = 0;
    while (i < qs_kv_size) {
      qs_kv[i] = substr_ptr;
      j = static_cast<int>(strcspn(substr_ptr, "&"));
      if (substr_ptr[j] == '\0') {
        break;
      }
      substr_ptr += j + 1;
      ++i;
    }
    ++i;  // x &'s -> means x iterations of this loop -> means *x+1* k/v pairs

    // we only decode the values in place, the keys could have '='s in them
    // which will hose our ability to distinguish keys from values later
    for (j = 0; j < i; ++j) {
      substr_ptr = qs_kv[j] + strcspn(qs_kv[j], "=&#");
      if (substr_ptr[0] == '&' ||
        substr_ptr[0] == '\0')  // blank value: skip decoding
        substr_ptr[0] = '\0';
      else
        qs_decode(++substr_ptr);
    }
    return i;
  }
  int qs_decode(char* qs) {
    int i = 0, j = 0;
    while (QS_ISQSCHR(qs[j])) {
      if (qs[j] == '+') {
        qs[i] = ' ';
      } else if (qs[j] == '%')  // easier/safer than scanf
      {
        if (!QS_ISHEX(qs[j + 1]) || !QS_ISHEX(qs[j + 2])) {
          qs[i] = '\0';
          return i;
        }
        qs[i] = (QS_HEX2DEC(qs[j + 1]) * 16) + QS_HEX2DEC(qs[j + 2]);
        j += 2;
      } else {
        qs[i] = qs[j];
      }
      ++i;
      ++j;
    }
    qs[i] = '\0';
    return i;
  }
  char* qs_k2v(const char* key, char* const* qs_kv, size_t qs_kv_size, int nth) {
    int i;
    size_t key_len, skip;
    key_len = strlen(key);
    for (i = 0; i < qs_kv_size; ++i) {
      // we rely on the unambiguous '=' to find the value in our k/v pair
      if (qs_strncmp(key, qs_kv[i], key_len) == 0) {
        skip = strcspn(qs_kv[i], "=");
        if (qs_kv[i][skip] == '=') ++skip;
        // return (zero-char value) ? ptr to trailing '\0' : ptr to value
        if (nth == 0)
          return qs_kv[i] + skip;
        else
          --nth;
      }
    }
    return nullptr;
  }
  int hex2dcolor(char* color, double* r, double* g, double* b, double* a) {
    int i, j;
    if (color == NULL) return 0;
    i = static_cast<int>(strlen(color));
    if (i != 8 && i != 6 && i != 4 && i != 3)  return 0;
    for (j = 0; j < i; j++)  if (!QS_ISHEX(color[j]))  return 0;
    switch (i) {
      // (H*16+H)/255 ==  H*17/255 == H/15
    case 3:
      *r = QS_HEX2DEC(color[0]) / 15.0;
      *g = QS_HEX2DEC(color[1]) / 15.0;
      *b = QS_HEX2DEC(color[2]) / 15.0;
      break;
    case 4:
      *r = QS_HEX2DEC(color[0]) / 15.0;
      *g = QS_HEX2DEC(color[1]) / 15.0;
      *b = QS_HEX2DEC(color[2]) / 15.0;
      *a = QS_HEX2DEC(color[3]) / 15.0;
      break;
    case 6:
      *r = ((QS_HEX2DEC(color[0]) * 16) + QS_HEX2DEC(color[1])) / 255.0;
      *g = ((QS_HEX2DEC(color[2]) * 16) + QS_HEX2DEC(color[3])) / 255.0;
      *b = ((QS_HEX2DEC(color[4]) * 16) + QS_HEX2DEC(color[5])) / 255.0;
      break;
    case 8:
      *r = ((QS_HEX2DEC(color[0]) * 16) + QS_HEX2DEC(color[1])) / 255.0;
      *g = ((QS_HEX2DEC(color[2]) * 16) + QS_HEX2DEC(color[3])) / 255.0;
      *b = ((QS_HEX2DEC(color[4]) * 16) + QS_HEX2DEC(color[5])) / 255.0;
      *a = ((QS_HEX2DEC(color[6]) * 16) + QS_HEX2DEC(color[7])) / 255.0;
      break;
    }
    return i;
  }
  int hex2ccolor(char* color, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a) {
    int i, j;
    if (color == NULL) return 0;
    i = static_cast<int>(strlen(color));
    if (i != 8 && i != 6 && i != 4 && i != 3)  return 0;
    for (j = 0; j < i; j++)  if (!QS_ISHEX(color[j]))  return 0;
    switch (i) {
      // (H*16+H) == H*17
    case 3:
      *r = QS_HEX2DEC(color[0]) * 17;
      *g = QS_HEX2DEC(color[1]) * 17;
      *b = QS_HEX2DEC(color[2]) * 17;
      break;
    case 4:
      *r = QS_HEX2DEC(color[0]) * 17;
      *g = QS_HEX2DEC(color[1]) * 17;
      *b = QS_HEX2DEC(color[2]) * 17;
      *a = QS_HEX2DEC(color[3]) * 17;
      break;
    case 6:
      *r = (QS_HEX2DEC(color[0]) * 16) + QS_HEX2DEC(color[1]);
      *g = (QS_HEX2DEC(color[2]) * 16) + QS_HEX2DEC(color[3]);
      *b = (QS_HEX2DEC(color[4]) * 16) + QS_HEX2DEC(color[5]);
      break;
    case 8:
      *r = (QS_HEX2DEC(color[0]) * 16) + QS_HEX2DEC(color[1]);
      *g = (QS_HEX2DEC(color[2]) * 16) + QS_HEX2DEC(color[3]);
      *b = (QS_HEX2DEC(color[4]) * 16) + QS_HEX2DEC(color[5]);
      *a = (QS_HEX2DEC(color[6]) * 16) + QS_HEX2DEC(color[7]);
      break;
    }
    return i;
  }
  std::unique_ptr<std::pair<std::string_view, std::string_view>> qs_dict_name2kv(const char* dict_name, char* const* qs_kv, size_t qs_kv_size, int nth) {
    int i;
    size_t name_len, skip_to_eq, skip_to_brace_open, skip_to_brace_close;
    name_len = strlen(dict_name);
    for (i = 0; i < qs_kv_size; ++i) {
      if (strncmp(dict_name, qs_kv[i], name_len) == 0) {
        skip_to_eq = strcspn(qs_kv[i], "=");
        if (qs_kv[i][skip_to_eq] == '=') ++skip_to_eq;
        skip_to_brace_open = strcspn(qs_kv[i], "[");
        if (qs_kv[i][skip_to_brace_open] == '[') ++skip_to_brace_open;
        skip_to_brace_close = strcspn(qs_kv[i], "]");
        if (skip_to_brace_open <= skip_to_brace_close && skip_to_brace_open > 0 &&
          skip_to_brace_close > 0 && nth == 0) {
          std::string_view key(qs_kv[i] + skip_to_brace_open, skip_to_brace_close - skip_to_brace_open);
          std::string_view value(qs_kv[i] + skip_to_eq);
          return std::unique_ptr<std::pair<std::string_view, std::string_view>>(new std::pair<std::string_view, std::string_view>(key, value));
        } else {
          --nth;
        }
      }
    }
    return nullptr;
  }
  char* qs_scanvalue(const char* key, const char* qs, char* val, size_t val_len) {
    size_t i, key_len;
    const char* tmp;
    // find the beginning of the k/v substrings
    if ((tmp = strchr(qs, '?')) != NULL) qs = tmp + 1;
    key_len = strlen(key);
    while (qs[0] != '#' && qs[0] != '\0') {
      if (qs_strncmp(key, qs, key_len) == 0) break;
      qs += strcspn(qs, "&") + 1;
    }
    if (qs[0] == '\0') return NULL;
    qs += strcspn(qs, "=&#");
    if (qs[0] == '=') {
      ++qs;
      i = strcspn(qs, "&=#");
#ifdef _MSC_VER
      strncpy_s(val, val_len, qs,
        (val_len - 1) < (i + 1) ? (val_len - 1) : (i + 1));
#else
      strncpy(val, qs, (val_len - 1) < (i + 1) ? (val_len - 1) : (i + 1));
#endif
      qs_decode(val);
    } else {
      if (val_len > 0) val[0] = '\0';
    }
    return val;
  }
  query_string::query_string(const query_string& qs) {
    for (char* p : qs.key_value_pairs_) key_value_pairs_.push_back(p);
  }
  query_string& query_string::operator=(const query_string& qs) {
    key_value_pairs_.clear(); for (auto p : qs.key_value_pairs_) key_value_pairs_.push_back(p); return *this;
  }
  query_string& query_string::operator=(query_string&& qs) {
    key_value_pairs_ = std::move(qs.key_value_pairs_); return *this;
  }
  query_string::query_string(std::string_view& url, size_t l) {
    if (url.empty()) return;
    key_value_pairs_.resize(MAX_KEY_VALUE_PAIRS_COUNT);
    int count = qs_parse(const_cast<char*>(&url[++l]), &key_value_pairs_[0], MAX_KEY_VALUE_PAIRS_COUNT);
    key_value_pairs_.resize(count);
  }
  std::string& operator<<(std::string& s, const query_string& qs) {
    s.append({ '[', ' ' });
    for (size_t i = 0; i < qs.key_value_pairs_.size(); ++i) {
      if (i) s.append({ ',', ' ' }); s << qs.key_value_pairs_[i];
    }
    s.append({ ' ', ']' });
    return s;
  }
  char* query_string::get(const char* name) const {
    char* ret = qs_k2v(name, key_value_pairs_.data(), key_value_pairs_.size()); return ret;
  }
  char* query_string::operator[](const char* name) const {
    char* ret = qs_k2v(name, key_value_pairs_.data(), key_value_pairs_.size()); return ret;
  }
  char* query_string::pop(const std::string_view& name) {
    char* ret = qs_k2v(name.data(), key_value_pairs_.data(), key_value_pairs_.size());
    if (ret != nullptr) {
      for (unsigned int i = 0; i < key_value_pairs_.size(); ++i) {
        std::string_view str_item(key_value_pairs_[i]);
        if (str_item.substr(0, name.size()) == name && str_item[name.size()] == '=') {
          key_value_pairs_.erase(key_value_pairs_.begin() + i);
          break;
        }
      }
    }
    return ret;
  }
  std::vector<char*> query_string::get_list(const std::string_view& name) const {
    std::vector<char*> ret;
    char* element = nullptr;
    int count = 0;
    std::string plus(name.data(), name.size());
    plus.append({ '[', ']' });
    while (1) {
      element = qs_k2v(plus.data(), key_value_pairs_.data(),
        key_value_pairs_.size(), count++);
      if (!element) break;
      ret.push_back(element);
    }
    return ret;
  }
  std::vector<char*> query_string::pop_list(const std::string_view& name) {
    std::vector<char*> ret = query_string::get_list(name);
    if (!ret.empty()) {
      for (unsigned int i = 0; i < key_value_pairs_.size(); ++i) {
        std::string_view str_item(key_value_pairs_[i]);
        if (str_item.substr(0, name.size()) == name && str_item.substr(name.size(), name.size() + 3) == "[]=") {
          key_value_pairs_.erase(key_value_pairs_.begin() + i--);
        }
      }
    }
    return ret;
  }
  std::unordered_map<std::string_view, std::string_view> query_string::get_dict(const std::string_view& name) const {
    std::unordered_map<std::string_view, std::string_view> ret;
    int count = 0;
    while (1) {
      if (auto element = qs_dict_name2kv(name.data(), key_value_pairs_.data(),
        key_value_pairs_.size(), count++))
        ret.insert(*element);
      else
        break;
    }
    return ret;
  }
  std::unordered_map<std::string_view, std::string_view> query_string::pop_dict(const std::string_view& name) {
    std::unordered_map<std::string_view, std::string_view> ret = get_dict(name);
    if (!ret.empty()) {
      for (unsigned int i = 0; i < key_value_pairs_.size(); ++i) {
        std::string_view str_item(key_value_pairs_[i]);
        if (str_item.substr(0, name.size()) == name && str_item[name.size()] == '[') {
          key_value_pairs_.erase(key_value_pairs_.begin() + i--);
        }
      }
    }
    return ret;
  }
  std::vector<std::string_view> query_string::keys() const {
    std::vector<std::string_view> ret;
    for (auto element : key_value_pairs_) {
      std::string_view str_element(element);
      ret.emplace_back(str_element.substr(0, str_element.find('=')));
    }
    return ret;
  }
} // namespace cc