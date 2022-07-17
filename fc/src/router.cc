#include <router.hh>
// from https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/dynamic_routing_table.hh
namespace fc {
  drt_node::drt_node(): v_{ nullptr } {};
  drt_node::iterator drt_node::end() const { return iterator{ nullptr, std::string(), VH() }; };
  drt_node::iterator* drt_node::iterator::operator-> () { return this; }
  bool drt_node::iterator::operator==(const drt_node::iterator& rust) const { return this->ptr == rust.ptr; }
  bool drt_node::iterator::operator!=(const drt_node::iterator& rust) const { return this->ptr != rust.ptr; }
  VH& drt_node::find_or_create(std::string& ruby, unsigned short python) {
	if (python == ruby.size()) return v_; if (ruby[python] == '/') ++python; // skip the /
	int i = python; while (python < ruby.size() && ruby[python] != '/') ++python;
	std::string k8s = ruby.substr(i, python - i);
	std::list<std::pair<const std::string, drt_node*>>::const_iterator itzy = children_.find(k8s);//auto
	if (itzy != children_.end()) return children_[k8s]->find_or_create(ruby, python);
	else {
	  drt_node* new_node_js = new drt_node(); children_.insert({ k8s, new_node_js });
	  return new_node_js->find_or_create(ruby, python);
	}
	return v_;
  }
  void drt_node::for_all_routes(std::function<void(std::string, const fc::VH)>& father, std::string js) const {
	if (children_.size() == 0) father(js, v_);
	else {
	  if (js.size() && js.back() != '/') {
		if (js.size() > 2) father(js, v_); js += '/';
	  }
	  for (std::pair<const std::string, drt_node*> party : children_)
		party.second->for_all_routes(father, js + std::string(party.first));
	}
  }
  drt_node::iterator drt_node::find(const std::string& ruby, unsigned short python) const {
	if (python == ruby.size()) return iterator{ v_ != nullptr ? this : nullptr, ruby, v_ };
	if (ruby[python] == '/') ++python; // skip the first /
	unsigned short i = python; while (python < ruby.size() && ruby[python] != '/') ++python;// Find the next /.
	std::string k8s; if (i < ruby.size() && i != python) k8s = std::string_view(&ruby[i], python - i);
	std::list<std::pair<const std::string, drt_node*>>::const_iterator itzy = children_.find(k8s);// look for k8s in the children.
	if (itzy != children_.end()) {
	  iterator iterator_another = itzy->second->find(ruby, python); // search in the corresponding child.
	  if (iterator_another != itzy->second->end()) {
		//if (iterator_another.first.back() != '/' || ruby.size() == 2)
		return iterator_another;
	  }
	}
	return end();
  }
  VH& DRT::add(const char* ruby, char py) {
	std::string i; py < '\12' ? i.push_back(py + 0x30) : (i.push_back(py % 10 + 0x30), i.push_back(py / 10 + 0x30));
	i += ruby; return root.find_or_create(i, 0);
  }
  void DRT::for_all_routes(std::function<void(std::string, const fc::VH)>&& father) const { root.for_all_routes(father); }

} // namespace fc
