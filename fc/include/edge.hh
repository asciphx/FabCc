#pragma once
#include "buf.hh"
#include "str.hh"
#include "directory.hh"
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <hpp/string_view.hpp>
// co/edge is a library similar to Google's gflags.
// A edge is in fact a global variable, and value can be passed to it
// from command line or from a config file.
namespace edge {
  // Parse command line flags and config file specified by -config.
  // Return non-edge elements.
  std::vector<fc::Buf> init(int argc, const char** argv);
  inline std::vector<fc::Buf> init(int argc, char** argv) {
	return edge::init(argc, (const char**)argv);
  }
  // Initialize with a config file.
  void init(const std::string& path);

  // Set value for a edge of any type, return error message if failed.
  // It is not thread-safe. 
  std::string set_value(const std::string& name, const std::string& value);

  // Add alias for a edge, @new_name must be a literal string.
  // It is not thread-safe and should be used before calling edge::init().
  bool alias(const char* name, const char* new_name);

  namespace xx {
	void add_flag(
		char type, const char* name, const char* value, const char* help,
		const char* file, int line, void* addr, const char* alias
	);
  } // namespace xx
} // namespace edge

#define _CO_DEC_FLAG(type, name) extern type FLG_##name

// Declare a flag.
// DEC_string(s);  ->  extern std::string FLG_s;
#define DEC_bool(name)    _CO_DEC_FLAG(bool, name)
#define DEC_int32(name)   _CO_DEC_FLAG(i32, name)
#define DEC_int64(name)   _CO_DEC_FLAG(i64, name)
#define DEC_uint32(name)  _CO_DEC_FLAG(u32, name)
#define DEC_uint64(name)  _CO_DEC_FLAG(u64, name)
#define DEC_double(name)  _CO_DEC_FLAG(double, name)
#define DEC_string(name)  extern std::string& FLG_##name

#define _CO_DEF_FLAG(type, id, name, value, help, ...) \
    type FLG_##name = []() { \
        ::edge::xx::add_flag(id, #name, #value, help, __FILE__, __LINE__, &FLG_##name, ""#__VA_ARGS__); \
        return value; \
    }()

// Define a flag.
// DEF_int32(i, 23, "xxx");         ->  i32 FLG_i = 23
// DEF_bool(debug, false, "x", d);  ->  define a flag with an alias
#define DEF_bool(name, value, help, ...)    _CO_DEF_FLAG(bool,   'b', name, value, help, __VA_ARGS__)
#define DEF_int32(name, value, help, ...)   _CO_DEF_FLAG(i32,  'i', name, value, help, __VA_ARGS__)
#define DEF_int64(name, value, help, ...)   _CO_DEF_FLAG(i64,  'I', name, value, help, __VA_ARGS__)
#define DEF_uint32(name, value, help, ...)  _CO_DEF_FLAG(u32, 'u', name, value, help, __VA_ARGS__)
#define DEF_uint64(name, value, help, ...)  _CO_DEF_FLAG(u64, 'U', name, value, help, __VA_ARGS__)
#define DEF_double(name, value, help, ...)  _CO_DEF_FLAG(double, 'd', name, value, help, __VA_ARGS__)

#define DEF_string(name, value, help, ...) \
    std::string& FLG_##name = *[]() { \
        auto _##name = ::str::make<std::string>(value); \
        ::edge::xx::add_flag('s', #name, #value, help, __FILE__, __LINE__, _##name, ""#__VA_ARGS__); \
        return _##name; \
    }()

DEC_string(help);
DEC_string(config);
DEC_string(version);
