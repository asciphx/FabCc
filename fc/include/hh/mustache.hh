#ifndef MUSTACHE_HPP
#define MUSTACHE_HPP
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <fstream>
#include <iterator>
#include "json.hh"
//from https://mustache.github.io/#demo
namespace fc {
  namespace mustache {
    class invalid_template_exception: public std::exception {
    public:
      invalid_template_exception(const std::string& msg);
      virtual const char* what() const throw();
      std::string msg;
    };

    enum class ActionType {
      Ignore, Tag, UnescapeTag, OpenBlock, CloseBlock, ElseBlock, Partial
    };

    struct Action {
      int start, end, pos;
      ActionType t;
      Action(ActionType t, int start, int end, int pos = 0);
    };

    std::string default_loader(const std::string& filename);

    class template_t {
    public:
      operator std::string() const;
      template_t(std::string body);
    private:
      std::vector<std::pair<int, int>> fragments_;
      std::vector<Action> actions_;
      std::string body_;
      std::string tag_name(const Action& action);
      auto find_context(const std::string& name, const std::list<const json::Json*>& stack) -> std::pair<bool, const json::Json&>;

      void escape(const std::string& in, std::string& out);

      void render_internal(int actionBegin, int actionEnd, std::list<const json::Json*>& stack, std::string& out, int indent);
      void render_fragment(const std::pair<int, int> fragment, int indent, std::string& out);
      void parse();
    public:
      std::string render();
      std::string render(const json::Json& ctx);
    };
    template_t load(std::string& filename);
    template_t load(const char* filename);
  }
}
#endif
