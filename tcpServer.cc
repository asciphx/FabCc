#include <tcp.hh>
#include <iostream>
using namespace fc;
void main() {
  App app;
  app.get() = [](Req&, Res& res) {
	res.write("hello world!你好！世界！");
  };
  app.get("/api") = [](Req&, Res& res) {
	res.write("lsdkagosjagojsdagklsdklgjsld");
  };
  Tcp srv(&app);
  //启动服务器
  srv.setTcpNoDelay(true).Start("127.0.0.1");
}