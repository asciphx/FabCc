# FabCc
Concise, fast, practical, reactive, functional. Inspired by other well-known c++ web frameworks.

## Features
- Can add, delete, modify and check the route
- Use the global timer task to control some things, such as shutting down the server when it expires
- With an API similar to nodejs, it also looks like JS
- Minimalist API, infinite possibilities

## Description
- The namespace uses the uppercase hump fc of FabCc.
- cmake plans to support Linux and MAC in the future. (cmakelists.txt has not been integrated yet)

## example
```c++
int main() {
  fc::Timer t; fc::App app; fc::Tcp srv;
  app.get() = [](Req&, Res& res) {
	res.write("hello world!你好！世界！这是主页！");
  };
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("Homepage route is deleted! Is now inaccessible！");
  };
  app["/api"] = [](Req&, Res& res) {
	res.write("lsdkagosjagojsdagklsdklgjsld");
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&] {
	  printf("The route has been idle for 1 minute, and the server will shut down automatically！！");
	  srv.exit();
	}, 60000);
	res.write("hello world!你好！！！");
	app.get() = [](Req&, Res& res) {
	  res.write("Home page route has been replicated, added or changed！");
	};
  };
  //Start the server
  srv.router(app).setTcpNoDelay(true).Start("127.0.0.1", 8080);
  return 0;
}
```
