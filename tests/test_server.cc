#include <fabCc.hh>
struct Person;
struct Book {
  std::string name = "BOOK!";
  box<Person> person; vec<Person> persons;
  REG
};
REGIS(Book, name, person, persons)
struct Person {
  std::string name;
  int age;
  box<Book> book; vec<Book> books;
  REG
};
REGIS(Person, name, age, book, books)
using namespace fc;
void funk(Req& req, Res& res) { res.write("Homepage route is replicated by std::bind！"); };
int main() {
  App app;
  app.file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4","webm","mp3","wav","mkv","srt","vtt" })
    .sub_api("/", app.serve_file("static")).set_keep_alive(4, 3, 2).set_use_max_mem(300.0)
    .set_file_download(true);//Set to enable file downloads, this is the new interface.
  // localhost:8080/params?foo=%27dg%27&pew=4&count[]=a&count[]=b&mydict[a]=b&mydict[abcd]=42
  app["/params"] = [](Req& req, Res& res) {
    std::string s;
    // To get a simple string from the url params
    // To see it in action /params?foo='blabla'
    s << "Params: " << req.params << "\n\n";
    s << "The key 'foo' was " << (req.params["foo"] == nullptr ? "not " : "") << "found.\n";
    // To get a double from the request
    // To see in action submit something like '/params?pew=42'
    if (req.params["pew"] != nullptr) {
      double countD = std::lexical_cast<double>(req.params["pew"]);
      s << "The value of 'pew' is " << countD << '\n';
    }
    // To get a list from the request
    // You have to submit something like '/params?count[]=a&count[]=b' to have a list with two values (a and b)
    auto count = req.params.get_list("count");
    s << "The key 'count' contains " << count.size() << " value(s).\n";
    for (const auto& countVal : count) {
      s << " - " << countVal << '\n';
    }
    // To get a dictionary from the request
    // You have to submit something like '/params?mydict[a]=b&mydict[abcd]=42' to have a list of pairs ((a, b) and (abcd, 42))
    auto mydict = req.params.get_dict("mydict");
    s << "The key 'dict' contains " << mydict.size() << " value(s).\n";
    for (const auto& mydictVal : mydict) {
      s << " - " << mydictVal.first << " -> " << mydictVal.second << '\n';
    }
    res.write(s);
  };
  app.default_route() = [](Req& req, Res& res) {
    res.set_content_type("text/html;charset=UTF-8", 23);
    res.write_async_s(req, [] {
      char name[64]; gethostname(name, 64);
      Json x{
        {"header", name},
        {"items", 
            {{{"name", "red"}, {"first", true}, {"url", "#Red"}},
            {{"name", "green"}, {"link", true}, {"url", "#Green"}},
            {{"name", "blue"}, {"link", true}, {"url", "#Blue"}}}
        },
        {"empty", false}
      };
      return mustache::load("404NotFound.html").render(x);
      });
  };
  app["/get_upload"] = [](Req& req, Res& res) {
    res.write_async(req, [] {
      auto f = fc::directory_iterator(fc::directory_ + fc::upload_path_); Json x;
      std::set<std::string_view> extentions = { "mp4", "mp3", "webm", "wav", "mkv" };
      for (auto v : f) {
        if (std::find(extentions.begin(), extentions.end(), fc::toLowerCase(v.ext)) != extentions.end()) {
          x.push_back({ {"name",v.name.substr(fc::directory_.size())}, {"size",v.size} });
        }
      } return x;
      });
  };
  app["/read_file"] = [](Req& req, Res& res) {
    res.write_async(req, [] { Json x = json::read_file("test.json"); return x; });
  };
  app["/json"] = [](Req& req, Res& res) {
    Json x; Book b{ "ts", Person{"js",6, Book{"plus" }, vec<Book>{ {"1", Person {"sb" }}, {"2", Person {"sb" }} }} };
    b.person->book = Book{ "rs", null, vec<Person>{ {"?"}, {"!"} } }; x = b; res.write(x);
  };
  app["/serialization"] = [](Req& req, Res& res) {
    Json x = json::parse(R"(
    {"name":"ts","person":{"name":"js","age":33,"book":{"name":"ojbk","person":{"name":"fucker","age":0},
    "persons":[{"name":"stupid","age":1},{"name":"idoit","age":2},{"name":"bonkers","age":3,"book":{"name":"sb"}}]}}}
    )"); Book b = x.get<Book>(); b.person->book->persons[2].name = "wwzzgg"; x = b; res.write(x.dump());
  };
  app["/api"] = [](Req& req, Res& res) {
    res.write(res.app._print_routes());//Return to routing list
  };
  app.post("/api") = [](Req& req, Res& res) {
    BP bp(req, 1000); std::string s;//Support for uploading files with a total size of 1000MB
    for (auto p : bp.params) {
      s << (p.key + ": ") << p.value << ", ";
    }
    s.pop_back(); s.pop_back(); res.write(s);
  };
  app["/del"] = [&app](Req&, Res& res) { app["/"] = nullptr; res.write("The routing of the home page is delete！！"); };
  app["/timer"] = [&](Req& req, Res& res) {
    req.setTimeout([] { raise(SIGINT); }, 6000);
    res.write("Turn off the server timer and start the countdown！");
    app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //Start the server, also supports ipv6
  app.http_serve(8080);
  //Start the server, only supports ipv4
  //app.http_serve(8080, "0.0.0.0");
}