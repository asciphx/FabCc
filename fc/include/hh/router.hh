#ifndef ROUTER_HH
#define ROUTER_HH
#include <unordered_map>
#include <string>
#include <memory>
#include <string>
#include <regex>
#include "h/common.h"
#include "hpp/hash_map.hpp"
#include "req-res.hh"
namespace fc {
  using VH = std::function<_CTX_TASK(void)(Req&, Res&)>; //class lambda [](std::string s, VH d)->void
  struct drt_node {
    drt_node();
    struct iterator {
      const drt_node* ptr; std::string first; VH second;
      iterator* operator-> ();
      bool operator==(const iterator& b) const;
      bool operator!=(const iterator& b) const;
    };
    VH& find_or_create(std::string& r, unsigned short c);
    void for_all_routes(std::function<void(std::string, const fc::VH)>& f, std::string prefix = "") const;
    iterator find(const std::string& r, unsigned short c) const;
    VH v_; std::unordered_map<std::string, drt_node*, str_hash, str_key_eq> children_;
  };
  static const drt_node::iterator DRT_END = drt_node::iterator{ nullptr, std::string(), VH{} };
  struct DRT {
    // Find a route && return reference to a procedure.
    VH& add(const char* r, char m);
    void for_all_routes(std::function<void(std::string, const fc::VH)>&& f) const;
    drt_node root;
  };
} // namespace li
#endif
