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
- 基于epoll架构的全平台支持[windows下由wepoll实现]
- 现在最低编译器版本支持到了c++14, 目前兼容了c++17的许多特性包括any, optional, string_view, 以及部分的扩展
- 最少的第三方库，均以源文件形式存放项目中
- 最快的api例如lexical_cast, 以及EncodeURL, DecodeURL
- 难以置信的编译速度，开发速度同时也得到提升
- 支持网页版Postman，地址是127.0.0.1:8080/test.html
- 可以对路由进行增删改查，动态路由参考【[lithium](https://github.com/matt-42/lithium)】迭代而成
- 支持类似于无栈协程的非对称协程, 像python的生成器, 协程来自【[boost](https://github.com/boostorg/context)】
- 拥有类似nodejs的api，写起来也像js, http解析器来自【[nodejs](https://github.com/nodejs/llhttp)】
- 字符串映射的str_map【[beast](https://github.com/boostorg/beast),[crow](https://github.com/ipkn/crow)】
- 静态文件缓存file_sptr【[libhttp](https://github.com/neithern/libhttp)】
- 支持单元测试，文档请见【[coost](https://coostdocs.gitee.io/cn/co/unitest/)】
- Json集成于【[coost](https://coostdocs.gitee.io/cn/co/json/)】, 并支持序列化与反序列化
- 设计具备借鉴于rust语言风格的box【[std::boxed::Box](https://doc.rust-lang.org/std/boxed/struct.Box.html)】，因此无需使用裸指针
- zlib压缩来自【[zlib](https://github.com/madler/zlib)】

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
  Timer t; App app;
  app.file_type().sub_api("/", app.serve_file("static"));//服务文件接口
  app["/json"] = [](Req& req, Res& res) {
	Json x = { { "h", 23 }, { "b", false }, { "s", "xx" }, { "v", {1,2,3} }, { "o", {{"xx", 0}} } };
	res.write(x.dump());//json响应
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//返回路由列表
  };
  app.post("/api") = [](Req& req, Res& res) {
	BP bp(req, 50);
	for (auto p : bp.params) {
	  res.body << (p.key + ": " + (!p.size ? p.value : p.filename) + ", ");
	}
	res.write(res.body);
  };
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("主页的路由被删除！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
    if(t.idle()) t.setTimeout([] { exit(0); }, 6000);
	res.write("关闭服务计时器倒计时启动！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  http_serve(app, 8080);
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
