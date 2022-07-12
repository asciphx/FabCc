#include <tcp.hh>
using namespace fc;
int main() {
  int i = 0;
  Timer t;
  App app;
  Tcp srv;
  app.get() = [](Req&, Res& res) {
	res.write("hello world!你好！世界！这是主页！");
  };
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页路由被删除！现已无法访问！");
  };
  app["/api"] = [](Req&, Res& res) {
	res.write("lsdkagosjagojsdagklsdklgjsld");
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&] {
	  printf("该路由已闲置1分钟，服务器即将自动关闭！！");
	  srv.exit();
	}, 60000);
	res.write("hello world!你好！！！");
	app.get() = [](Req&, Res& res) {
	  res.write("主页路由已复写，被添加或被更改！");
	};
  };
  //启动服务器
  srv.router(app).setTcpNoDelay(true).Start("127.0.0.1", 8080);
  return 0;
}