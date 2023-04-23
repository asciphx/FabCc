#include <app.hh>
#include <hpp/body_parser.hpp>
#include <json.hh>
#include <timer.hh>
struct Person;
struct Book {
  std::string name = "BOOK!";
  box<Person> person; vec<Person> persons;
  REG(Book, name, person, persons)
};
CLASS(Book, name, person, persons)
struct Person {
  std::string name;
  int age;
  box<Book> book; vec<Book> books;
  REG(Person, name, age, book, books)
};
CLASS(Person, name, age, book, books)
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("Homepage route is replicated by std::bind！");
};
int main() {
  App app; Timer t;
  app.file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4" })
    .sub_api("/", app.serve_file("static")).set_use_max_mem(300.0);//Service file interface
  app["/json"] = [](Req& req, Res& res) {
    Book b{ "ts", Person{"js",33, Book{ "plus" }, vec<Book>{ Book{"1", Person { "sb1" }},Book{"2", Person { "sb2" }} }} };
    b.person->book = Book{ "rs" };//Write C++ like Object-Oriented Programming
    Json x; to_json(x, &b); x["person"]["book"]["person"] = b.person; res.write(x.dump());
  };
  app["/serialization"] = [](Req& req, Res& res) {
    Json x; Book b; from_json(x = json::parse(R"(
	{"name":"ts","person":{"name":"js","age":33,"book":{"name":"ojbk","person":{"name":"fucker","age":0},
	"persons":[{"name":"stupid","age":1},{"name":"idoit","age":2},{"name":"bonkers","age":3,"book":{"name":"sb"}}]}}}
	)"), &b); b.person->book->persons[2].name = "wwzzgg"; to_json(x, &b); res.write(x.dump());
  };
  app["/api"] = [&app](Req& req, Res& res) {
    res.write(app._print_routes());//Return to routing list
  };
  app.post("/api") = [](Req& req, Res& res) {
    BP bp(req, 1000);//Support for uploading files with a total size of 1000MB
    for (auto p : bp.params) {
      res.body << (p.key + ": ") << (!p.is_file ? p.value : p.filename) << ", ";
    }
    res.write(res.body.pop_back().pop_back());
  };
  app["/del"] = [&app](Req&, Res& res) {
    app.get() = nullptr;
    res.write("The routing of the home page is delete！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
    if (t.idle()) t.setTimeout([] { raise(SIGINT); }, 6000);
    res.write("Turn off the server timer and start the countdown！");
    app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //Start the server
  http_serve(app, 8080);
}