#include <app.hh>
#include <hpp/body_parser.hpp>
#include <json.hh>
#include <timer.hh>
struct Person;
struct Book {
  fc::Buf name = "Hello, world!";
  box<Person> person; vec<Person> persons;
  REG(Book, name, person, persons)
};
CLASS(Book, name, person, persons)
struct Person {
  fc::Buf name;
  int age;
  box<Book> book; vec<Book> books;
  REG(Person, name, age, book, books)
};
CLASS(Person, name, age, book, books)
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("主页路由被std::bind复写！");
};
int main() {
  App app; Timer t;
  app.file_type().sub_api("/", app.serve_file("static"));//服务文件接口
  app["/json"] = [](Req& req, Res& res) {
	Json x; Book b{ "ts", box<Person>{"plus",23, box<Book>{"js", box<Person>{"ds"}}, vec<Book>{ Book{},Book{} }} };
	to_json(x, &b); x["person"]["book"]["person"]["book"] = box<Book>(b);
	res.write(x.dump());//json响应
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app.post("/api") = [](Req& req, Res& res) {
	BP bp(req, 50);
	for (auto p : bp.params) {
	  res.body << (p.key + ": " + (!p.size ? p.value : p.filename) + ", ");
	}
	res.write(res.body);
  };
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页的路由被删除！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
	if (t.idle()) t.setTimeout([] { raise(SIGINT); }, 6000);
	res.write("6秒后关闭服务！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  http_serve(app, 8080);
}