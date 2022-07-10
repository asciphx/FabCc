# FabCc
简洁、迅速、实用、活跃、全面。灵感来自于其他c++知名web框架。

## 説明
- 命名空間采用FabCc的大寫駝峰fc來使用。
- 暫時只提供了基於libuv的windows版本的服務端開發，後續打算支持linux，mac。
- 暫時由于buffer需要缓冲，所以得先刷新几次页面预热，才能测试，因此需要进行线程池改造，但会更加复杂。
- ![单线程测试](./single_core_test.jpg)

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
