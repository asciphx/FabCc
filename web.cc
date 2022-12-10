#include <tcp.hh>
#include <hpp/body_parser.hpp>
#include <json.hh>
#include <tp/ctx.hh>
using namespace fc;
struct Person;
struct Book {
  fc::Buf name = "Hello, world!";
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
int main() {
  Timer t; App app; Tcp srv;
  app.sub_api("/", app.serve_file("static"));//服务文件接口
  app["/json"] = [](Req& req, Res& res) {
	Json x; Book b{ "ts", box<Person>{"plus",23, box<Book>{"js", box<Person>{"ds"}}, vec<Book>{ Book{},Book{} }} };
	to_json(x, &b); x.get("person").get("book").get("person").get("book") = box<Book>(b);
	res.write(x.dump());//json响应
  };
  app["/sockets"] = [&srv](Req& req, Res& res) {
	Buf b("("); b << srv.$.size() << ")[";
	for (std::set<u32>::iterator i = srv.$.begin(); i != srv.$.end(); ++i) b << *i << ',';
	res.write(b.pop_back() << ']');//获取所有活动的套接字id
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app.post("/api") = [](Req& req, Res& res) {
	BP bp(req, 4096);
	for (auto p : bp.params) {
	  res.write(p.key + ": " + (!p.size ? p.value : p.filename) + ", ");
	}
  };
  app["/yield"] = [](Req& req, Res& res) {
	Json x = { 1,2,3 };
	co c{ [&x](co&& c) {
	  x = json::parse(R"([{"confidence":0.974220335483551,"text":"lenovo联想","region":[[191,80],[672,80],[672,148],[191,148]]},
		{"confidence":0.6968730688095093,"text":"BY：花享湖月","region":[[250,866],[332,866],[332,885],[250,885]]}])");
	  return std::move(c);
	  } };
	res.write(x.str());
	c = c.yield();
	res.write(x.dump());
  };//协程, 使用co的yield函数来保证执行顺序
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页的路由被删除！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&srv] {
	  printf("该路由已闲置1分钟，服务器即将自动关闭！！");
	  srv.exit();
	}, 60000);
	res.write("关闭服务计时器倒计时启动！");
  };
  //启动服务器
  srv.router(app).timeout(9000).setTcpNoDelay(true).setThread().Start("0.0.0.0", 8080);
  return 0;
}