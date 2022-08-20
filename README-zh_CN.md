# FabCc
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
简洁、迅速、实用、活跃、全面。灵感来自于其他c++知名web框架, 堪称c++版的nodejs。

## 特征
- 可以对路由进行增删改查
- 能够使用全局定时器任务，来控制一些事情，比如到期关闭服务器
- 拥有类似nodejs的api，写起来也像js
- 极简的api，无限的可能
- 最快的api例如lexical_cast, 以及EncodeURL, DecodeURL
- 难以置信的编译速度，开发速度同时也得到提升
- 最少的第三方库，均以源文件形式存放项目中
- 全平台支持，（已经测试Linux和Windows）
- 最小化分配与释放内存，对硬盘友好，不产生大量内存碎片，因此几乎没有磁盘io
- 支持单元测试，文档请见【[coost](https://coostdocs.gitee.io/cn/co/unitest/)】
- 支持网页版Postman，地址是127.0.0.1:8080/test.html

## 仍在开发中
- [x] 路由大括号表达式
- [x] gzip压缩
- [x] body-parser的支持
- [ ] ssl证书
- [ ] websocket
- [ ] udp服务端
- [ ] tcp客户端client

## 説明
- 如果机器翻译过来，FabCc可以是晶圆厂，所以FabCc象征着最复杂又最小化的芯片。
- 命名空間采用FabCc的大寫駝峰fc來使用。
- logo还没来得及更换，暂时是用的crow的logo。
- regex表达式路由，因为性能的问题将被移除。
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
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app.post("/api") = [](Req& req, Res& res) {
	BP bp(req, 4096);
	for (auto p : bp.params) {
	  res.write(p.key + ": " + (!p.size ? p.value : p.filename) + ", ");
	}
  };
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
  srv.router(app).timeout(6000).setTcpNoDelay(true).Start("0.0.0.0", 8080);
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
