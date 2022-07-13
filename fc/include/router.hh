#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <memory>
#include <string>
#include <h/common.h>
#include <req-res.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/dynamic_routing_table.hh
namespace fc {
  using VH = std::function<void(Req&, Res&)>;
  //class lambda [](std::string s, VH d)->void
  struct drt_node {
	drt_node(): v_{ nullptr } {}//*
	struct iterator {
	  const drt_node* ptr; std::string first; VH second;
	  iterator* operator-> () { return this; }
	  bool operator==(const iterator& b) const { return this->ptr == b.ptr; }
	  bool operator!=(const iterator& b) const { return this->ptr != b.ptr; }
	};
	iterator end() const { return iterator{ nullptr, std::string(), VH() }; }
	VH& find_or_create(std::string& r, unsigned short c) {
	  if (c == r.size()) return v_; if (r[c] == '/') ++c; // skip the /
	  int s = c; while (c < r.size() && r[c] != '/') ++c;
	  std::string k = r.substr(s, c - s);
	  std::list<std::pair<const std::string, drt_node*>>::const_iterator it = children_.find(k);//auto
	  if (it != children_.end())
		return children_[k]->find_or_create(r, c);
	  else {
		drt_node* new_node = new drt_node();
		children_.insert({ k, new_node });
		return new_node->find_or_create(r, c);
	  }
	  return v_;
	}
	void for_all_routes(std::function<void(std::string, const fc::VH)>& f, std::string prefix = "") const {
	  if (children_.size() == 0) f(prefix, v_);
	  else {
		if (prefix.size() && prefix.back() != '/') prefix += '/';
		for (std::pair<const std::string, drt_node*> pair : children_)
		  pair.second->for_all_routes(f, prefix + std::string(pair.first));
	  }
	}
	iterator find(const std::string& r, unsigned short c) const {
	  if ((c == r.size() && v_ != nullptr) || (children_.size() == 0))// We found the route r.
		return iterator{ this, r, v_ };
	  if (c == r.size() && v_ == nullptr)// r does not match any route.
		return iterator{ nullptr, r, v_ };
	  if (r[c] == '/') ++c; // skip the first /
	  unsigned short s = c; while (c < r.size() && r[c] != '/') ++c;// Find the next /.
	  std::string k; if (s < r.size() && s != c) k = std::string_view(&r[s], c - s);
	  std::list<std::pair<const std::string, drt_node*>>::const_iterator it = children_.find(k);// look for k in the children.
	  if (it != children_.end()) {
		iterator it2 = it->second->find(r, c); // search in the corresponding child.
		if (it2 != it->second->end()) return it2;
	  }
	  {
		// if one child is a url param {{param_name}}, choose it
		for (std::pair<std::string, drt_node*> kv : children_) {
		  std::string& name = kv.first;
		  if (name.size() > 4 && name[0] == '{' && name[1] == '{' &&
			name[name.size() - 2] == '}' && name[name.size() - 1] == '}')
			return kv.second->find(r, c);
		}
		return end();
	  }
	}
	VH v_; std::unordered_map<std::string, drt_node*> children_;
  };
  struct DRT {
	// Find a route && return reference to a procedure.
	VH& add(const char* r, char m) {
	  std::string s; s.reserve(0x1f); m > 9 ? s.push_back(m % 10 + 0x30), s.push_back(m / 10 + 0x30) :
		s.push_back(m + 0x30); s += r; return root.find_or_create(s, 0);
	}
	drt_node::iterator find(const std::string& r) const { return root.find(r, 0); }
	void for_all_routes(std::function<void(std::string, const fc::VH)>&& f) const { root.for_all_routes(f); }
	drt_node::iterator end() const { return root.end(); }
	drt_node root;
  };

} // namespace li
