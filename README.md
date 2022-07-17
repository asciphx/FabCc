# FabCc
Concise, fast, practical, reactive, functional. Inspired by other well-known c++ web frameworks.

## Features
- Can add, delete, modify and check the route
- Use the global timer task to control some things, such as shutting down the server when it expires
- With an API similar to nodejs, it also looks like JS
- Minimalist API, infinite possibilities
- Fastest API, such as lexical_cast, EncodeURL, DecodeURL

## Description
- The namespace uses the uppercase hump fc of FabCc.
- cmake plans to support Linux and MAC in the future. (cmakelists.txt has not been integrated yet)

## example
```c++
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("Homepage route is replicated by std::bind！");
};
int main() {
  Timer t; App app; Tcp srv;
  app.get() = [](Req&, Res& res) {
	res.write("hello world!你好！世界！这是主页！");
  };
  app["/api"] = [&app](Req&, Res& res) {
	res.write(app._print_routes().c_str());//Return to routing list
  };
  app.post("/api") = [](Req&, Res& res) {
	res.write("This is the post method！");
  };
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = [](Req&, Res& res) { res.code = 403; };
	res.write("The routing of the home page is disabled！！");//Or bind the void method in the following std::bind way
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&srv] {
	  printf("The route has been idle for 1 minute, and the server will shut down automatically！！");
	  srv.exit();
	}, 60000);
	res.write("Timer countdown start！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //Start the server
  srv.router(app).timeout(4000).setTcpNoDelay(true).Start("127.0.0.1", 8080);
  return 0;
}
```
