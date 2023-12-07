#include "hh/mustache.hh"
#include "h/common.h"
#include "hh/lexical_cast.hh"

namespace fc {
  namespace mustache {
    invalid_template_exception::invalid_template_exception(const std::string& msg): msg("fc::mustache error: " + msg) {}
    const char* invalid_template_exception::what() const throw() { return msg.c_str(); }

    Action::Action(ActionType t, int start, int end, int pos): start(start), end(end), pos(pos), t(t) {}

    std::string default_loader(const std::string& filename) {
      std::string path = fc::directory_ + filename; std::ifstream inf(path); if (!inf) return {};
      return { std::istreambuf_iterator<char>(inf), std::istreambuf_iterator<char>() };
    }

    template_t::operator std::string() const { return body_; };
    template_t::template_t(std::string body)
      : body_(std::move(body)) {
      // {{ {{# {{/ {{^ {{! {{> {{=
      parse();
    }
    std::string template_t::tag_name(const Action& action) {
      return body_.substr(action.start, action.end - action.start);
    }
    auto template_t::find_context(const std::string& name, const std::list<const json::Json*>& stack) -> std::pair<bool, const json::Json&> {
      if (name == ".") {
        return { true, *stack.back() };
      }
      int dotPosition = static_cast<int>(name.find("."));
      if (dotPosition == -1) {
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
          if ((*it)->type() == json::Json::t_object) {
            if ((*it)->count(name))
              return { true, (**it)[name] };
          }
        }
      } else {
        std::vector<int> dotPositions;
        dotPositions.push_back(-1);
        while (dotPosition != -1) {
          dotPositions.push_back(static_cast<int>(dotPosition));
          dotPosition = static_cast<int>(name.find(".", dotPosition + 1));
        }
        dotPositions.push_back(static_cast<int>(name.size()));
        std::vector<std::string> names;
        names.reserve(dotPositions.size() - 1);
        for (int i = 1; i < static_cast<int>(dotPositions.size()); ++i)
          names.emplace_back(name.substr(dotPositions[i - 1] + 1, dotPositions[i] - dotPositions[i - 1] - 1));

        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
          const json::Json* view = *it;
          bool found = true;
          for (auto jt = names.begin(); jt != names.end(); ++jt) {
            if (view->type() == json::Json::t_object &&
              view->count(*jt)) {
              view = &(*view)[*jt];
            } else {
              found = false;
              break;
            }
          }
          if (found)
            return { true, *view };
        }
      }
      return { false, json::empty_str };
    }

    void template_t::escape(const std::string& in, std::string& out) {
      out.reserve(out.size() + in.size());
      for (auto it = in.begin(); it != in.end(); ++it) {
        switch (*it) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"': out += "&quot;"; break;
        case '/': out += "&#x2F;"; break;
        case '\'': out += "&#39;"; break;
        default: out += *it; break;
        }
      }
    }

    void template_t::render_internal(int actionBegin, int actionEnd, std::list<const json::Json*>& stack, std::string& out, int indent) {
      int current = actionBegin;
      if (indent) out.insert(out.size(), indent, ' ');
      while (current < actionEnd) {
        auto& fragment = fragments_[current];
        auto& action = actions_[current];
        render_fragment(fragment, indent, out);
        switch (action.t) {
        case ActionType::Ignore:
          // do nothing
          break;
        case ActionType::Partial:
        {
          std::string partial_name = tag_name(action);
          auto partial_templ = template_t(default_loader(partial_name));
          int partial_indent = action.pos;
          partial_templ.render_internal(0, static_cast<int>(partial_templ.fragments_.size()) - 1, stack, out, partial_indent ? indent + partial_indent : 0);
        }
        break;
        case ActionType::UnescapeTag:
        case ActionType::Tag:
        {
          auto optional_ctx = find_context(tag_name(action), stack);
          auto& ctx = optional_ctx.second;
          switch (ctx.type()) {
          case json::Json::t_int:
          case json::Json::t_uint:
            out += ctx.dump();
            break;
          case json::Json::t_string: {
            std::string ss{ ctx.dump() };
            if (action.t == ActionType::Tag)
              escape(ss.substr(1, ss.size() - 2), out);
            else
              out += ss;
          } break;
          default:
            throw std::runtime_error("not implemented tag type" + std::lexical_cast<std::string>(ctx.type()));
          }
        }
        break;
        case ActionType::ElseBlock:
        {
          auto optional_ctx = find_context(tag_name(action), stack);
          if (!optional_ctx.first) {
            stack.emplace_back(&json::nullContext);
            break;
          }

          auto& ctx = optional_ctx.second;
          switch (ctx.type()) {
          case json::Json::t_array:
            if (ctx.array_size())
              current = action.pos;
            else
              stack.emplace_back(&json::nullContext);
            break;
          case json::Json::t_bool:
            //case json::Json::t_null:
            stack.emplace_back(&json::nullContext);
            break;
          default:
            current = action.pos;
            break;
          }
          break;
        }
        case ActionType::OpenBlock:
        {
          auto optional_ctx = find_context(tag_name(action), stack);
          if (!optional_ctx.first) {
            current = action.pos;
            break;
          }
          auto& ctx = optional_ctx.second;
          switch (ctx.type()) {
          case json::Json::t_array: {
            if (ctx.array_size())
              for (auto it = ctx.begin(); it != ctx.end(); ++it) {
                stack.push_back(&*it);
                render_internal(current + 1, action.pos, stack, out, indent);
                stack.pop_back();
              }
            current = action.pos;
          } break;
          case json::Json::t_int:
          case json::Json::t_uint:
          case json::Json::t_double:
          case json::Json::t_string:
          case json::Json::t_object:
            stack.push_back(&ctx);
            break;
          case json::Json::t_bool:
            //case json::Json::t_null:
            if (ctx.as_bool() == false) {
              current = action.pos;
            } else
              stack.push_back(&ctx);
            break;
          default:
            throw std::runtime_error("{{#: not implemented context type: " + std::lexical_cast<std::string>(ctx.type()));
          }
          break;
        }
        case ActionType::CloseBlock:
          stack.pop_back();
          break;
        default:
          throw std::runtime_error("not implemented " + std::lexical_cast<std::string>(static_cast<int>(action.t)));
        }
        ++current;
      }
      auto& fragment = fragments_[actionEnd];
      render_fragment(fragment, indent, out);
    }
    void template_t::render_fragment(const std::pair<int, int> fragment, int indent, std::string& out) {
      if (indent) {
        for (int i = fragment.first; i < fragment.second; ++i) {
          out += body_[i];
          if (body_[i] == '\n' && i + 1 != (int)body_.size())
            out.insert(out.size(), indent, ' ');
        }
      } else out.insert(out.size(), body_, fragment.first, fragment.second - fragment.first);
    }

    void template_t::parse() {
      std::string tag_open("{{", 2);
      std::string tag_close("}}", 2);
      std::list<int> blockPositions;
      size_t current = 0;
      while (1) {
        size_t idx = body_.find(tag_open, current);
        if (idx == body_.npos) {
          fragments_.emplace_back(static_cast<int>(current), static_cast<int>(body_.size()));
          actions_.emplace_back(ActionType::Ignore, 0, 0);
          break;
        }
        fragments_.emplace_back(static_cast<int>(current), static_cast<int>(idx));

        idx += tag_open.size();
        size_t endIdx = body_.find(tag_close, idx);
        if (endIdx == idx) {
          throw invalid_template_exception("empty tag is not allowed");
        }
        if (endIdx == body_.npos) {
          // error, no matching tag
          throw invalid_template_exception("not matched opening tag");
        }
        current = endIdx + tag_close.size();
        switch (body_[idx]) {
        case '#':
          ++idx;
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx - 1] == ' ') --endIdx;
          blockPositions.emplace_back(static_cast<int>(actions_.size()));
          actions_.emplace_back(ActionType::OpenBlock, static_cast<int>(idx), static_cast<int>(endIdx));
          break;
        case '/':
          ++idx;
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx - 1] == ' ') --endIdx;
          {
            auto& matched = actions_[blockPositions.back()];
            if (body_.compare(idx, endIdx - idx,
              body_, matched.start, matched.end - matched.start) != 0) {
              throw invalid_template_exception("not matched {{# {{/ pair: " +
                body_.substr(matched.start, matched.end - matched.start) + ", " +
                body_.substr(idx, endIdx - idx));
            }
            matched.pos = static_cast<int>(actions_.size());
          }
          actions_.emplace_back(ActionType::CloseBlock, static_cast<int>(idx), static_cast<int>(endIdx), blockPositions.back());
          blockPositions.pop_back();
          break;
        case '^':
          ++idx;
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx - 1] == ' ') --endIdx;
          blockPositions.emplace_back(static_cast<int>(actions_.size()));
          actions_.emplace_back(ActionType::ElseBlock, static_cast<int>(idx), static_cast<int>(endIdx));
          break;
        case '!':
          // do nothing action
          actions_.emplace_back(ActionType::Ignore, static_cast<int>(idx + 1), static_cast<int>(endIdx));
          break;
        case '>': // partial
          ++idx;
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx - 1] == ' ') --endIdx;
          actions_.emplace_back(ActionType::Partial, static_cast<int>(idx), static_cast<int>(endIdx));
          break;
        case '{':
          if (tag_open != "{{" || tag_close != "}}")
            throw invalid_template_exception("cannot use triple mustache when delimiter changed");

          ++idx;
          if (body_[endIdx + 2] != '}') {
            throw invalid_template_exception("{{{: }}} not matched");
          }
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx - 1] == ' ') --endIdx;
          actions_.emplace_back(ActionType::UnescapeTag, static_cast<int>(idx), static_cast<int>(endIdx));
          ++current;
          break;
        case '&':
          ++idx;
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx - 1] == ' ') --endIdx;
          actions_.emplace_back(ActionType::UnescapeTag, static_cast<int>(idx), static_cast<int>(endIdx));
          break;
        case '=':
          // tag itself is no-op
          ++idx;
          actions_.emplace_back(ActionType::Ignore, static_cast<int>(idx), static_cast<int>(endIdx));
          --endIdx;
          if (body_[endIdx] != '=')
            throw invalid_template_exception("{{=: not matching = tag: " + body_.substr(idx, endIdx - idx));
          --endIdx;
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx] == ' ') --endIdx;
          ++endIdx;
          {
            bool succeeded = false;
            for (size_t i = idx; i < endIdx; ++i) {
              if (body_[i] == ' ') {
                tag_open = body_.substr(idx, i - idx);
                while (body_[i] == ' ') ++i;
                tag_close = body_.substr(i, endIdx - i);
                if (tag_open.empty())
                  throw invalid_template_exception("{{=: empty open tag");
                if (tag_close.empty())
                  throw invalid_template_exception("{{=: empty close tag");

                if (tag_close.find(" ") != tag_close.npos)
                  throw invalid_template_exception("{{=: invalid open/close tag: " + tag_open + " " + tag_close);
                succeeded = true;
                break;
              }
            }
            if (!succeeded)
              throw invalid_template_exception("{{=: cannot find space between new open/close tags");
          }
          break;
        default:
          // normal tag case;
          while (body_[idx] == ' ') ++idx;
          while (body_[endIdx - 1] == ' ') --endIdx;
          actions_.emplace_back(ActionType::Tag, static_cast<int>(idx), static_cast<int>(endIdx));
        }
      }

      // removing standalones
      for (int i = static_cast<int>(actions_.size()) - 2; i >= 0; --i) {
        if (actions_[i].t == ActionType::Tag || actions_[i].t == ActionType::UnescapeTag)
          continue;
        auto& fragment_before = fragments_[i];
        auto& fragment_after = fragments_[i + 1];
        bool is_last_action = i == (int)actions_.size() - 2;
        bool all_space_before = true;
        int j, k;
        for (j = fragment_before.second - 1; j >= fragment_before.first; --j) {
          if (body_[j] != ' ') {
            all_space_before = false;
            break;
          }
        }
        if (all_space_before && i > 0)
          continue;
        if (!all_space_before && body_[j] != '\n')
          continue;
        bool all_space_after = true;
        for (k = fragment_after.first; k < (int)body_.size() && k < fragment_after.second; ++k) {
          if (body_[k] != ' ') {
            all_space_after = false;
            break;
          }
        }
        if (all_space_after && !is_last_action)
          continue;
        if (!all_space_after &&
          !(body_[k] == '\n' ||
            (body_[k] == '\r' &&
              k + 1 < (int)body_.size() &&
              body_[k + 1] == '\n')))
          continue;
        if (actions_[i].t == ActionType::Partial) {
          actions_[i].pos = fragment_before.second - j - 1;
        }
        fragment_before.second = j + 1;
        if (!all_space_after) {
          if (body_[k] == '\n')
            ++k;
          else
            k += 2;
          fragment_after.first = k;
        }
      }
    }
    std::string template_t::render() {
      json::Json empty_ctx;
      std::list<const json::Json*> stack;
      stack.emplace_back(&empty_ctx);

      std::string ret;
      render_internal(0, static_cast<int>(fragments_.size()) - 1, stack, ret, 0);
      return ret;
    }
    std::string template_t::render(const json::Json& ctx) {
      std::list<const json::Json*> stack;
      stack.emplace_back(&ctx);

      std::string ret;
      render_internal(0, static_cast<int>(fragments_.size()) - 1, stack, ret, 0);
      return ret;
    }

    template_t load(std::string& filename) { return template_t(default_loader(filename)); }
    template_t load(const char* filename) { return template_t(default_loader(filename)); }
  }
}