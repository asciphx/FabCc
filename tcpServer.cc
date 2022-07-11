#include <tcp.hh>
using namespace fc;
int main() {
  Timer t;
  App app;
  app.get() = [](Req&, Res& res) {
	res.write("hello world!你好！世界！");
  };
  app["/api"] = [](Req&, Res& res) {
	res.write("lsdkagosjagojsdagklsdklgjsld");
  };
  app["/timer"] = [&t](Req&, Res& res) {
	t.setTimeout([] {
	  printf("1秒后打印！！！");
	}, 1000);
	res.write("hello world!你好！！！");
  };
  Tcp srv(&app);
  //启动服务器
  srv.setTcpNoDelay(true).Start("127.0.0.1", 8080);
  return 0;
}