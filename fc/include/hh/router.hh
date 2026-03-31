#ifndef ROUTER_HH
#define ROUTER_HH
#include <unordered_map>
#include <string>
#include <memory>
#include <string>
#include <regex>
#include <functional>
#include "h/common.h"
#include "hpp/hash_map.hpp"
#include "req-res.hh"
namespace fc {
  // using VH = _CTX_TASK(void)(*)(Req&, Res&);
  struct drt_node {
    drt_node();
    struct iterator {
      const drt_node* ptr; std::string first; _CTX_TASK(void)(*second)(Req&, Res&);
      iterator* operator-> ();
      bool operator==(const iterator& b) const;
      bool operator!=(const iterator& b) const;
    };
    _CTX_TASK(void) (*&find_or_create(std::string& r, unsigned short c))(Req&, Res&);
    void for_all_routes(void(*& f)(void*, std::string, _CTX_TASK(void)(*)(Req&, Res&)), void* a, std::string prefix = "") const;
    iterator find(const std::string& r, unsigned short c) const;
    _CTX_TASK(void)(*v_)(Req&, Res&); std::unordered_map<std::string, drt_node*, str_hash, str_key_eq> children_;
  };
  static const drt_node::iterator DRT_END = drt_node::iterator{ nullptr, std::string(), (_CTX_TASK(void)(*)(Req&, Res&))nullptr };
  struct DRT {
    // Find a route && return reference to a procedure.
    _CTX_TASK(void) (*&add(const char* r, char m))(Req&, Res&);
    void for_all_routes(void(*&& f)(void*, std::string, _CTX_TASK(void)(*)(Req&, Res&)), void* arg) const;
    drt_node root;
  };
} // namespace li
#endif
