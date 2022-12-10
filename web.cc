#include <app.hh>
//#include <hpp/body_parser.hpp>
#include <json.hh>
#include <tp/ctx.hh>
#include <timer.hh>
struct Person;
struct Book {
  fc::Buf name = "wtf";
  box<Person> person;
  vec<Person> persons;
  REG(Book, name, person, persons)
};
CLASS(Book, name, person, persons)
struct Person {
  fc::Buf name;
  int age;
  box<Book> book;
  vec<Book> books;
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
	to_json(x, &b); x.get("person").get("book").get("person").get("book") = box<Book>(b);
	res.write(res.body << x.dump());//json响应
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app["/yield"] = [](Req& req, Res& res) {
	Json x = { 1,2,3 };
	co c{ [&x](co&& c) {
	  x = json::parse(R"([{"confidence":0.974220335483551,"text":"lenovo联想","region":[[191,80],[672,80],[672,148],[191,148]]},
		{"confidence":0.6968730688095093,"text":"BY：花享湖月","region":[[250,866],[332,866],[332,885],[250,885]]}])");
	  return std::move(c);
	  } };
	res.body << x;
	c = c.yield();
	res.body << x.dump();
	res.write(res.body);
  };//协程, 使用co的yield函数来保证执行顺序
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