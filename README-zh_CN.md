# FabCc
简洁、迅速、实用、活跃、全面。灵感来自于其他c++知名web框架。

## 特征
- 可以对路由进行增删改查
- 能够使用全局定时器任务，来控制一些事情，比如到期关闭服务器
- 拥有类似nodejs的api，写起来也像js
- 极简的api，无限的可能

## 説明
- 命名空間采用FabCc的大寫駝峰fc來使用。
- cmake方面後續打算支持linux，mac。(CMakeLists.txt还没集成)

## 例子
```c++
int main() {
  fc::Timer t; fc::App app; fc::Tcp srv;
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
```
