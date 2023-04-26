# FabCc(1.4-RTM)
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
灵感来自于其他c++知名web框架, FabCC的定位是一个网络框架, 其特点是低代码, 高性能, 强类型, 超标准, 最安全, 很牛逼。

## [Eng](./README.md) | 简中
> 已发布的版本才是稳定版，而分支的可能无法在某些平台编译成功，因为分支是开发版。

## 原创
- 从0-1设计具备借鉴于rust语言风格的box【[std::boxed::Box](https://doc.rust-lang.org/std/boxed/struct.Box.html)】  
  ，因此不需要使用原始指针，C++也可以用OOP方式编写
- 由box引申出并从0-1完美解决了循环依赖的问题，而且超级智能指针box可以替代optional，兼容了std::optional
- 目前的box也可以替代原始指针，从0-1自动管理内存，无需考虑内存释放，相当于自动GC，旨在将c++脚本化
- body_parser用于处理multi_part文件上传，不单支持单文件支持自定义大小，甚至还可以支持多文件上传并结合了mmap的零拷贝的存储映射
- lexical_cast用于从字符串，字符视图到基本类型之间的互相转换，从0-1实现的是将header-only变为头文件源文件分离，并加速了编译速度
- router采用了动态路由技术，可以增删改查，目前还支持使用正则表达式，这是其他框架恰恰不具备的
- 设计了Buf类，用于在某些时候替代string使用，还提供了reset方法，用于复用内存而不是像clear那样的低效
- string_view用于补足在c++14以及更旧的版本中无法使用string_view的痛点，并且最大程度的扩展了新版中的功能
- text是从0-1用于解决utf8的字符串存储，并且可以自行设定容量，每个容量都是针对utf8的，可以应对数据库中的varchar类型
- timer是从0-1用于解决c++没有javascript计时器的问题的，并且才用了头文件源文件分离，加速了编译速度
- 以上部件是个人技术实力的展示。当然，也有一些部件，其中并不完全是从0到1，但那些不是最重要的部件。

## 特征
- 基于epoll架构的全平台支持[windows下由wepoll实现]
- 现在最低编译器版本支持到了c++14, 目前兼容了c++17的许多特性包括any, optional, string_view, 以及部分的扩展
- 最少的第三方库，均以源文件形式存放项目中
- 最快的api例如lexical_cast, 以及EncodeURL, DecodeURL, itoa
- 难以置信的编译速度，开发速度同时也得到提升
- 支持网页版Postman，地址是127.0.0.1:8080/test.html
- 可以对路由进行增删改查，动态路由参考【[lithium](https://github.com/matt-42/lithium)】迭代而成
- 支持类似于无栈协程的非对称协程, 像python的生成器, 协程来自【[boost](https://github.com/boostorg/context)】
- 拥有类似nodejs的api，写起来也像js, http解析器来自【[nodejs](https://github.com/nodejs/llhttp)】
- 字符串映射的str_map【[beast](https://github.com/boostorg/beast),[crow](https://github.com/ipkn/crow)】
- 静态文件缓存file_sptr【[libhttp](https://github.com/neithern/libhttp)】
- 支持单元测试，文档请见【[coost](https://coostdocs.gitee.io/cn/co/unitest/)】
- Json集成于【[coost](https://coostdocs.gitee.io/cn/co/json/)】, 并支持序列化与反序列化
- zlib压缩来自【[zlib](https://github.com/madler/zlib)】
- mman来自【[mman](https://code.google.com/archive/p/mman-win32/source/default/source)】
- cache_file来自【[drogon](https://github.com/drogonframework/drogon/blob/master/lib/src/CacheFile.cc)】

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
  App app; Timer t;
  app.file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4" })
    .sub_api("/", app.serve_file("static"));//服务文件接口
  app["/json"] = [](Req& req, Res& res) {
    Book b{ "ts", Person{"js",33, Book{ "plus" }, vec<Book>{ Book{"1", Person { "sb1" }},Book{"2", Person { "sb2" }} }} };
    b.person->book = Book{ "rs" };//C++中的面向对象开发
    Json x; to_json(x, &b); x["person"]["book"]["person"] = b.person; res.write(x.dump());
  };
  app["/serialization"] = [](Req& req, Res& res) {
    Json x; Book b; from_json(x = json::parse(R"(
	{"name":"ts","person":{"name":"js","age":33,"book":{"name":"ojbk","person":{"name":"fucker","age":0},
	"persons":[{"name":"stupid","age":1},{"name":"idoit","age":2},{"name":"bonkers","age":3,"book":{"name":"sb"}}]}}}
	)"), &b); b.person->book->persons[2].name = "wwzzgg"; to_json(x, &b); res.write(x.dump());//反序列化与序列化
  };
  app["/api"] = [&app](Req& req, Res& res) {
	  res.write(app._print_routes());//返回路由列表
  };
  app.post("/api") = [](Req& req, Res& res) {
    BP bp(req, 1000);// 支持上传的文件总大小1000MB
    for (auto p : bp.params) {
      res.body << (p.key + ": ") << (!p.is_file ? p.value : p.filename) << ", ";
    }
    res.write(res.body);
  };
  app["/del"] = [&app](Req&, Res& res) {
    app.get() = nullptr;
    res.write("The routing of the home page is delete！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
    if (t.idle()) t.setTimeout([] { raise(SIGINT); }, 6000);
	  res.write("关闭服务计时器倒计时启动！");
    app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //启动服务器
  http_serve(app, 8080);
}
```

### 建筑（测试、示例）
建议使用CMake进行源代码外构建。
如果构建失败，请删除清理cmake缓存。
```
mkdir build
cd build
cmake ..
cmake --build ./ --config Release
```
