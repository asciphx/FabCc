#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <string>
#include <regex>
#include <h/common.h>
#include <req-res.hh>
#include <buffer.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/dynamic_routing_table.hh
namespace fc {
  using VH = std::function<void(Req&, Res&)>; //class lambda [](std::string s, VH d)->void
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
	iterator find(const std::string& r, unsigned short c, Req* request) const;
	VH v_; std::unordered_map<std::string, drt_node*> children_;
  };
  static const drt_node::iterator DRT_END = drt_node::iterator{ nullptr, std::string(), VH() };
  struct DRT {
	// Find a route && return reference to a procedure.
	VH& add(const char* r, char m);
	void for_all_routes(std::function<void(std::string, const fc::VH)>&& f) const;
	drt_node root;
  };

} // namespace li
