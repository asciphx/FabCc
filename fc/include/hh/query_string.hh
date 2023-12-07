#ifndef QUERY_STRING_HH
#define QUERY_STRING_HH
#include <string.h>
#include <unordered_map>
#include <vector>
#include <hpp/string_view.hpp>
#include <memory>
#include <iostream>
namespace cc {
  // ----------------------------------------------------------------------------
  // qs_parse (modified)
  // https://github.com/bartgrantham/qs_parse
  // ----------------------------------------------------------------------------
  /*  Similar to strncmp, but handles URL-encoding for either string  */
  int qs_strncmp(const char* s, const char* qs, size_t n);
  /*  Finds the beginning of each key/value pair and stores a pointer in qs_kv.
  *  Also decodes the value portion of the k/v pair *in-place*.  In a future
  *  enhancement it will also have a compile-time option of sorting qs_kv
  *  alphabetically by key.  */
  int qs_parse(char* qs, char* qs_kv[], int qs_kv_size);
  /*  Used by qs_parse to decode the value portion of a k/v pair  */
  int qs_decode(char* qs);
  /*  Looks up the value according to the key on a pre-processed query string
  *  A future enhancement will be a compile-time option to look up the key
  *  in a pre-sorted qs_kv array via a binary search.  */
  // char * qs_k2v(const char * key, char * qs_kv[], int qs_kv_size);
  char* qs_k2v(const char* key, char* const* qs_kv, size_t qs_kv_size, int nth = 0);
  /*  Non-destructive lookup of value, based on key.  User provides the
  *  destinaton string and length.  */
  char* qs_scanvalue(const char* key, const char* qs, char* val, size_t val_len);
  std::unique_ptr<std::pair<std::string_view, std::string_view>> qs_dict_name2kv(const char* dict_name, char* const* qs_kv, size_t qs_kv_size, int nth = 0);
  /*  Converts the 3 or 6 (RGB), or 4 or 8 (RGBA) hex chars in the color string
  *  to double values in the range 0.0-1.0.  Returns the number of converted chars.  */
  int hex2dcolor(char* color, double* r, double* g, double* b, double* a);
  /*  Converts the 3/6 (RGB) or 4/8 (RGBA) hex chars in the color string to
  *  values spanning the full range of unsigned char, 0-255.  Returns the number of converted chars.  */
  int hex2ccolor(char* color, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a);
  /// A class to represent any data coming after the `?` in the Req URL into
  /// key-value pairs.
  struct query_string {
    static const int MAX_KEY_VALUE_PAIRS_COUNT = 32;
    query_string() {}
    query_string(const query_string& qs);
    query_string& operator=(const query_string& qs);
    query_string& operator=(query_string&& qs);
    query_string(std::string_view& url, size_t l);
    _FORCE_INLINE void clear() { key_value_pairs_.clear(); }
    friend std::string& operator<<(std::string& s, const query_string& qs);
    /// Get a value from a name, used for `?name=value`.
    ///
    /// Note: this method returns the value of the first occurrence of the key
    /// only, to return all occurrences, see \ref get_list().
    char* operator[](const char* name) const;
    char* get(const char* name) const;
    /// Works similar to \ref get() except it removes the item from the query
    /// string.
    char* pop(const std::string_view& name);
    /// Returns a list of values, passed as
    /// `?name[]=value1&name[]=value2&...name[]=valuen` with n being the size of
    /// the list.
    ///
    /// Note: Square brackets in the above example are controlled by
    /// `use_brackets` boolean (true by default). If set to false, the example
    /// becomes `?name=value1,name=value2...name=valuen`
    std::vector<char*> get_list(const std::string_view& name) const;
    /// Similar to \ref get_list() but it removes the
    std::vector<char*> pop_list(const std::string_view& name);
    /// Works similar to \ref get_list() except the brackets are mandatory must
    /// not be empty.
    ///
    /// For example calling `get_dict(yourname)` on
    /// `?yourname[sub1]=42&yourname[sub2]=84` would give a map containing `{sub1
    /// : 42, sub2 : 84}`.
    ///
    /// if your query string has both empty brackets and ones with a key inside,
    /// use pop_list() to get all the values without a key before running this
    /// method.
    std::unordered_map<std::string_view, std::string_view> get_dict(const std::string_view& name) const;
    /// Works the same as \ref get_dict() but removes the values from the query
    /// string.
    std::unordered_map<std::string_view, std::string_view> pop_dict(const std::string_view& name);
    std::vector<std::string_view> keys() const;
  private:
    std::vector<char*> key_value_pairs_;
  };
}  // namespace cc
#endif
