# FabCc
简洁、迅速、实用、活跃、全面。灵感来自于其他c++知名web框架。

## 説明
- 命名空間采用FabCc的大寫駝峰fc來使用。
- cmake方面後續打算支持linux，mac。(CMakeLists.txt还没集成)

## 例子
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
  //启动服务器
  srv.setTcpNoDelay(true).Start("127.0.0.1", 8080);
  return 0;
}
```
