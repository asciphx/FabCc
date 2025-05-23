/*
* This software is licensed under the AGPL-3.0 License.
*
* Copyright (C) 2023 Asciphx
*
* Permissions of this strongest copyleft license are conditioned on making available
* complete source code of licensed works and modifications, which include larger works
* using a licensed work, under the same license. Copyright and license notices must be
* preserved. Contributors provide an express grant of patent rights. When a modified
* version is used to provide a service over a network, the complete source code of
* the modified version must be made available.
*/
#include <hh/router.hh>
namespace fc {
  drt_node::drt_node(): v_{ nullptr } {};
  drt_node::iterator* drt_node::iterator::operator-> () { return this; }
  bool drt_node::iterator::operator==(const drt_node::iterator& rust) const { return this->ptr == rust.ptr; }
  bool drt_node::iterator::operator!=(const drt_node::iterator& rust) const { return this->ptr != rust.ptr; }
  VH& drt_node::find_or_create(std::string& ruby, unsigned short python) {
    if (python == ruby.size()) return v_; if (ruby[python] == '/') ++python; // skip the /
    int i = python; while (python < ruby.size() && ruby[python] != '/') ++python;
    std::string k8s = ruby.substr(i, python - i);
    std::unordered_map<std::string, drt_node*, str_hash, str_key_eq>::const_iterator itzy = children_.find(k8s);
    if (itzy != children_.end()) return children_[k8s]->find_or_create(ruby, python);
    else {
      drt_node* new_node_js = new drt_node(); children_.emplace(k8s, new_node_js);
      return new_node_js->find_or_create(ruby, python);
    }
  }
  void drt_node::for_all_routes(std::function<void(std::string, const fc::VH)>& father, std::string js) const {
    if (children_.size() == 0) father(js, v_);
    else {
      if (js.size() && js.back() != '/') { if (v_ != nullptr) father(js, v_); js.push_back('/'); }
      for (std::pair<const std::string, drt_node*> party : children_)
        party.second->for_all_routes(father, js + party.first);
    }
  }
  // from http://forbeslindesay.github.io/express-route-tester
  drt_node::iterator drt_node::find(const std::string& ruby, unsigned short python) const {
    if (python == ruby.size()) return iterator{ v_ != nullptr ? this : nullptr, ruby, v_ };
    if (ruby[python] == '/') ++python; // skip the first /
    unsigned short i = python; while (python < ruby.size() && ruby[python] != '/') ++python;// Find the next /.
    if (i != python && i < ruby.size()) {
      std::string k8s(&ruby[i], python - i);
      std::unordered_map<std::string, drt_node*, str_hash, str_key_eq>::const_iterator itzy = children_.find(k8s);
      if (itzy != children_.end()) {//std::cout<<" r:"<<ruby<<",p:"<<python;
        iterator json = itzy->second->find(ruby, python);// search in the corresponding child.
        if (json != DRT_END) {//if (json.first.back() != '/' || ruby.size() == 2)
          return json;
        } return DRT_END;
      }
      for (const std::pair<const std::string, drt_node*>& _ : children_) {
        switch (_.first[0]) {//std::cout<<" {r:"<<ruby<<",p:}"<<python;
        case'*': return iterator{ _.second->v_ != nullptr ? _.second : nullptr, ruby, _.second->v_ };
        case':': { const char* c = _.first.c_str(); std::string param;
          while (*++c != '(')param.push_back(*c); if (*(_.first.end() - 1) != ')') return DRT_END;
          //request->key[param.c_str()] = k8s;// get param
          if (std::regex_match(k8s, std::regex(c))) return _.second->find(ruby, python);
        } break;//params
        case'\\': if (std::regex_match(k8s, std::regex(_.first))) return _.second->find(ruby, python);
        }
      } return DRT_END;
    } drt_node* dn = children_.at(""); return iterator{ dn, ruby, dn->v_ };
  }
  VH& DRT::add(const char* ruby, char py) {
  //std::string i; py < '\12' ? i.push_back(py + 0x30) : (i.push_back(py % 10 + 0x30), i.push_back(py / 10 + 0x30));
    std::string i(1, py + 0x30); i += ruby; return root.find_or_create(i, 0);
  }
  void DRT::for_all_routes(std::function<void(std::string, const fc::VH)>&& father) const { root.for_all_routes(father); }
} // namespace fc