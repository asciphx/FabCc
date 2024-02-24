#include <fabCc.hh>
struct Person;
struct Book {
  std::string name = "BOOK!";
  box<Person> person; vec<Person> persons;
  Book(std::string n = "", box<Person> p = null, vec<Person> ps = {}): name(n), person(p), persons(ps) {}//C++11
  REG
};
REGIS(Book, name, person, persons)
struct Person {
  std::string name;
  int age;
  box<Book> book; vec<Book> books;
  Person(std::string n = "", int a = 0, box<Book> b = null, vec<Book> bs = {}): name(n), age(a), book(b), books(bs) {}//C++11
  REG
};
REGIS(Person, name, age, book, books)
using namespace fc;
void funk(Req& req, Res& res) { res.write("Homepage route is replicated by std::bind！"); };
int main() {
  App app;
  app.file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4","webm","mp3","wav","mkv","srt","vtt" })
    .sub_api("/", app.serve_file("static")).set_keep_alive(4, 3, 2).set_use_max_mem(600.0)
    .set_file_download(true);//Set to enable file downloads, this is the new interface.
  app.default_route() = [](Req& req, Res& res) {
    res.set_content_type("text/html;charset=UTF-8", 23);
    res.write_async_s([] {
      char name[64]; gethostname(name, 64); Json x{ {"header", name} }; return mustache::load("404NotFound.html").render(x);
      });
  };
  app["/get_upload"] = [](Req& req, Res& res) {
    res.write_async([] {
      auto f = fc::directory_iterator(fc::directory_ + fc::upload_path_); Json x;
      std::set<std::string_view> extentions = { "mp4", "mp3", "webm", "wav", "mkv" };
      for (auto v : f) {
        if (std::find(extentions.begin(), extentions.end(), fc::toLowerCase(v.ext)) != extentions.end()) {
          x.push_back({ {"name",v.name.substr(fc::directory_.size())}, {"size",v.size} });
        }
      } return x;
      });
  };
  app["/read_file"] = [](Req& req, Res& res) { res.write_async([] { Json x = json::read_file("test.json"); return x; }); };
  app["/json"] = [](Req& req, Res& res) {
    Json x; Book b{ "ts", Person{"js",6, Book{"plus" }, vec<Book>{ {"1", Person {"sb" }}, {"2", Person {"mojo!🔥🔥" }} }} };
    b.person->book = Book{ "rs", null, vec<Person>{ {"?"}, {"!"} } }; x = b; res.write(x);
  };
  app["/serialization"] = [](Req& req, Res& res) {
    Json x = json::parse(R"(
    {"name":"ts","person":{"name":"٩(•̤̀ᵕ•̤́๑)ᵒᵏᵎ","age":33,"book":{"name":"😂ojbk","person":{"name":"fucker🤣","age":0},
    "persons":[{"name":"stupid","age":1},{"name":"idoit","age":2},{"name":"bonkers","age":3,"book":{"name":"sb"}}]}}}
    )"); Book b = x.get<Book>(); b.person->book->persons[2].name = "CRAZIERRR🔥"; x = b; res.write(x.dump());
  };
  app["/api"] = [](Req& req, Res& res) { res.write(res.app._print_routes()); };
  app.post("/api") = [](Req& req, Res& res) {
    BP bp(req, 1000); std::string s;//Support for uploading files with a total size of 1000MB
    for (auto p : bp.params) {
      s << (p.key + ": ") << p.value << ", ";
    }
    s.pop_back(); s.pop_back(); res.write(s);
  };
  app["/del"] = [](Req&, Res& res) { res.app["/"] = nullptr; res.write("The routing of the home page is delete！！"); };
  app["/timer"] = [](Req& req, Res& res) {
    req.setTimeout([] { raise(SIGINT); }, 6000); res.write("Turn off the server timer and start the countdown！");
    res.app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //Start the server
  app.http_serve(8080, "0.0.0.0");
}