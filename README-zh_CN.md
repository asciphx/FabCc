# FabCc
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
灵感来自于其他c++知名web框架, FabCC的定位是一个网络框架, 其特点是简洁、迅速、实用、活跃、全面。

## 特征
- 现在最低编译器版本支持到了c++14, 目前兼容了c++17的许多特性包括any, optional, string_view, 以及部分的扩展
- 可以对路由进行增删改查
- 能够使用全局定时器任务，来控制一些事情，比如到期关闭服务器
- 支持类似于无栈协程的非对称协程, 像python的生成器, 提供了yield方法用于转移执行
- 拥有类似nodejs的api，写起来也像js
- 极简的api，无限的可能
- 最快的api例如lexical_cast, 以及EncodeURL, DecodeURL
- 难以置信的编译速度，开发速度同时也得到提升
- 最少的第三方库，均以源文件形式存放项目中
- 全平台支持，（已经测试Linux和Windows）
- 最小化分配与释放内存，对硬盘友好，不产生大量内存碎片，因此几乎没有磁盘io
- 支持单元测试，文档请见【[coost](https://coostdocs.gitee.io/cn/co/unitest/)】
- 支持网页版Postman，地址是127.0.0.1:8080/test.html
- Json集成于【[coost](https://coostdocs.gitee.io/cn/co/json/)】, 并支持序列化与反序列化
- 具备设计借鉴于rust语言风格的box，因此无需使用裸指针

## 仍在开发中
- [x] 路由大括号表达式
- [x] gzip压缩
- [x] body-parser的支持
- [ ] ssl证书
- [ ] websocket
- [x] 协程
- [ ] udp服务端
- [ ] tcp客户端client

## 説明
- 如果机器翻译过来，FabCc可以是晶圆厂，所以FabCc象征着最复杂又最小化的芯片。
- 命名空間采用FabCc的大寫駝峰fc來使用。
- logo还没来得及更换，暂时是用的crow的logo。
- 回环测试之前需要先刷新几次网页页面预热, 然后wrk测试就不会崩溃, 具体原因未知。
- 协程的原理图
- ![yield](./yield.gif)
- [示例](http://8.129.58.72:8080/)🚀
- ![测试](./test.jpg)

## 例子
```c++
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("主页路由被std::bind复写！");
};
int main() {
  Timer t; App app; Tcp srv;
  app.sub_api("/", app.serve_file("static"));//服务文件接口
  app["/json"] = [](Req& req, Res& res) {
	Json x = { { "h", 23 }, { "b", false }, { "s", "xx" }, { "v", {1,2,3} }, { "o", {{"xx", 0}} } };
	res.write(x.dump());//json响应
  };
  app["/sockets"] = [&srv](Req& req, Res& res) {
	Buf b("("); b << srv.$.size() << ")[";
	for (std::set<u32>::iterator i = srv.$.begin(); i != srv.$.end(); ++i) b << *i << ',', Conn::shut(*i, _READ);
	res.write(b.pop_back() << ']');//获取所有活动的套接字id, 并且把所有套接字关闭
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app.post("/api") = [](Req& req, Res& res) {
	BP bp(req, 4096);
	for (auto p : bp.params) {
	  res.write(p.key + ": " + (!p.size ? p.value : p.filename) + ", ");
	}
  };
  app["/yield"] = [](Req& req, Res& res) {
	Json x = { 1,2,3 };
	co c{ [&x](co&& c) {
	  x = json::parse(R"([{"confidence":0.974220335483551,"text":"lenovo联想","region":[[191,80],[672,80],[672,148],[191,148]]},
		{"confidence":0.6968730688095093,"text":"BY：花享湖月","region":[[250,866],[332,866],[332,885],[250,885]]}])");
	  return std::move(c);
	  } };
	res.write(x.str());
	c = c.yield();
	res.write(x.dump());
  };//协程, 使用co的yield函数来保证执行顺序
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页的路由被删除！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&srv] {
	  printf("该路由已闲置1分钟，服务器即将自动关闭！！");
	  srv.exit();
	}, 60000);
	res.write("关闭服务计时器倒计时启动！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  srv.router(app).timeout(6000).setTcpNoDelay(true).setThread().Start("0.0.0.0", 8080);
  return 0;
}
```

### 建筑（测试、示例）
建议使用CMake进行源代码外构建。
```
mkdir build
cd build
cmake ..
cmake --build ./ --config Release
```
