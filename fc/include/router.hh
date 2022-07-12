#pragma once
#include <unordered_map>
#include <map>
#include <string_view>
#include <memory>
#include <string>
#include <h/common.h>
#include <req-res.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/dynamic_routing_table.hh
namespace fc {
  struct VH { char verb = static_cast<char>(HTTP::GET); std::function<void(Req&, Res&)> handler; std::string url; };
  namespace internal {//class lambda [](std::string s, VH d)->void
	struct drt_node {
	  drt_node(): v_{ 0, nullptr } {}//*
	  struct iterator {
		const drt_node* ptr; std::string_view first; VH second;
		iterator* operator-> () { return this; }
		bool operator==(const iterator& b) const { return this->ptr == b.ptr; }
		bool operator!=(const iterator& b) const { return this->ptr != b.ptr; }
	  };
	  iterator end() const { return iterator{ nullptr, std::string_view(), VH() }; }
	  VH& find_or_create(std::string_view r, unsigned int c) {
		if (c == r.size()) return v_; if (r[c] == '/') ++c; // skip the /
		int s = c; while (c < r.size() && r[c] != '/') c++;
		std::string_view k = r.substr(s, c - s);
		std::list<std::pair<const std::string_view, drt_node*>>::const_iterator it = children_.find(k);//auto
		if (it != children_.end())
		  return children_[k]->find_or_create(r, c);
		else {
		  drt_node* new_node = new drt_node();
		  children_.insert({ k, new_node });
		  return new_node->find_or_create(r, c);
		}
		return v_;
	  }
	  void for_all_routes(std::function<void(std::string,const fc::VH)>& f, std::string prefix = "") const {
		if (children_.size() == 0) f(prefix, v_);
		else {
		  if (prefix.size() && prefix.back() != '/') prefix += '/';
		  for (std::pair<const std::string_view, drt_node*> pair : children_)
			pair.second->for_all_routes(f, prefix + std::string(pair.first));
		}
	  }
	  iterator find(const std::string_view& r, unsigned int c) const {
		if ((c == r.size() && v_.handler != nullptr) || (children_.size() == 0))// We found the route r.
		  return iterator{ this, r, v_ };
		if (c == r.size() && v_.handler == nullptr)// r does not match any route.
		  return iterator{ nullptr, r, v_ };
		if (r[c] == '/') c++; // skip the first /
		int s = c; while (c < r.size() && r[c] != '/') c++;// Find the next /.
		std::string_view k(&r[s], c - s);// k is the string between the 2 /.
		std::list<std::pair<const std::string_view, drt_node*>>::const_iterator it = children_.find(k);// look for k in the children.
		if (it != children_.end()) {
		  iterator it2 = it->second->find(r, c); // search in the corresponding child.
		  if (it2 != it->second->end()) return it2;
		}
		{
		  // if one child is a url param {{param_name}}, choose it
		  for (std::pair<const std::string_view, drt_node*> kv : children_) {
			std::string_view name = kv.first;
			if (name.size() > 4 && name[0] == '{' && name[1] == '{' &&
			  name[name.size() - 2] == '}' && name[name.size() - 1] == '}')
			  return kv.second->find(r, c);
		  }
		  return end();
		}
	  }
	  VH v_; std::unordered_map<std::string_view, drt_node*> children_;
	};
  } // namespace internal
  struct DRT {
	// Find a route && return reference to a procedure.
	VH& operator[](const std::string_view& r) {
	  strings.push_back(std::make_shared<std::string>(r));
	  std::string_view r2(*strings.back());
	  return root.find_or_create(r2, 0);
	}
	VH& operator[](const std::string& r) {
	  strings.push_back(std::make_shared<std::string>(r));
	  std::string_view r2(*strings.back());
	  return root.find_or_create(r2, 0);
	}
	// Find a route && return an iterator.
	internal::drt_node::iterator find(const std::string_view& r) const { return root.find(r, 0); }
	void for_all_routes(std::function<void(std::string,const fc::VH)>&& f) const { root.for_all_routes(f); }
	internal::drt_node::iterator end() const { return root.end(); }
	std::vector<std::shared_ptr<std::string>> strings;
	internal::drt_node root;
  };

} // namespace li
