# FabCc
简洁、迅速、实用、活跃、全面。灵感来自于其他c++知名web框架。

## 特征
- 可以对路由进行增删改查
- 能够使用全局定时器任务，来控制一些事情，比如到期关闭服务器
- 拥有类似nodejs的api，写起来也像js
- 极简的api，无限的可能
- 最快的api例如lexical_cast, 以及EncodeURL, DecodeURL

## 説明
- 命名空間采用FabCc的大寫駝峰fc來使用。
- cmake方面後續打算支持linux，mac。(CMakeLists.txt还没集成)

## 例子
```c++
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("主页路由被std::bind复写！");
};
int main() {
  Timer t; App app; Tcp srv;
  app.get() = [](Req&, Res& res) {
	res.write("hello world!你好！世界！这是主页！");
  };
  app["/api"] = [&app](Req&, Res& res) {
	res.write(app._print_routes().c_str());//返回路由列表
  };
  app.post("/api") = [](Req&, Res& res) {
	res.write("这是post方法！");
  };
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = [](Req&, Res& res) { res.code = 403; };
	res.write("主页的路由被禁止！！");//或者像下面std::bind的方式把void方法绑定都行
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&srv] {
	  printf("该路由已闲置1分钟，服务器即将自动关闭！！");
	  srv.exit();
	}, 60000);
	res.write("计时器倒计时启动！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  srv.router(app).timeout(4000).setTcpNoDelay(true).Start("127.0.0.1", 8080);
  return 0;
}
```
