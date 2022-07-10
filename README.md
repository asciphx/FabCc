# FabCc
Concise, fast, practical, reactive, functional. Inspired by other well-known c++ web frameworks.
## Description
- The namespace uses the uppercase hump fc of FabCc.
- cmake plans to support Linux and MAC in the future. (cmakelists.txt has not been integrated yet)

## example
```c++
int main() {
  fc::App app;
  app.get() = [](Req&, Res& res) {
	res.write("hello world!你好！世界！");
  };
  app.get("/api") = [](Req&, Res& res) {
	res.write("lsdkagosjagojsdagklsdklgjsld");
  };
  fc::Tcp srv(&app);
  //Start the server
  srv.setTcpNoDelay(true).Start("127.0.0.1", 8080);
  return 0;
}
```
