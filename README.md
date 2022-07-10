# FabCc
Concise, fast, practical, reactive, functional. Inspired by other well-known c++ web frameworks.
## Description
- The namespace uses the uppercase hump fc of FabCc.
- For the time being, it only provides the server development of Windows version based on libuv, and plans to support Linux and MAC in the future.
- -For the time being, the buffer needs to be buffered, so you have to refresh the page several times to warm up before testing. Therefore, you need to transform the thread pool, but it will be more complex.
- ![single thread test](./single_core_test.jpg)

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
