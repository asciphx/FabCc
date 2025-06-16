# FabCc(v1.0-UHD)
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
灵感来自于其他c++知名web框架, FabCc的定位是一个网络框架, 其特点是低代码, 高性能, 强类型, 超标准, 最安全, 很牛逼。logo采用Nod的logo，设计来自命令与征服（CNC）。

![FabCc](./static/logo.png)
## [Eng](./README.md) | 简中
> 4月12日，超高清8k重制版到来。支持Gzip压缩网页降低流量消耗。修复各种bug，兼容了现代json、c++11，修复了keep-alive保活机制, 推出最强c++20无栈协程，以下是对比图。
> ![coroutine](./co%20vs%20Task.jpg)

## 原创
- 全面深度整合RBTree和时间轮（提升5%性能）封装的可取消定时器，同时修改底层协程用黑魔法管理其内存的释放，从而节省了内存
- 支持c++20无栈协程，目前兼容了原项目的有栈非对称协程，并且是完美对接，几乎只需要少量的宏改动, 性能比有栈协程强了5%左右。
- 增强型字段反射，例如`std::string_view sv = k(&O::id);`将返回"`O`.`id`",(在C++14及更高版本可用constexpr修饰)。
- 基于openssl的tcp客户端，由于是初步支持，尚且功能有限，大部分测试也可以通过。
- 支持jsonc格式，以便兼容注释。不过，这只是在读取json文件的时候才会调用。
- 拥有现代json的语法，以及对几乎所有的stl容器的全面兼容，还有全方位的宏来进行编译期的序列化与反序列化。
- 支持MSVC上面的多核编译选项，使用`cmake --build ./build --config Release -j`既可并行编译。
- 支持8K超高清视频播放功能，无卡顿，无延迟, 并且解锁超过4GB大小的视频的播放（亮点）。
- 支持Gzip压缩功能，默认缓存了6秒，在CACHE_HTML_TIME_SECOND宏配置修改既可。之所以设置压缩比率为6，是因为个人认为这是效率与压缩比的最优解。
- 全平台支持video或者是audio格式的range请求，可以随意点播跳转到任意的播放点, 例如wav,mp3,mp4,webm。并且可以设置是否允许下载，然后还支持暂停或者继续下载。
- 从0-1设计具备借鉴于rust语言风格的box【[std::boxed::Box](https://doc.rust-lang.org/std/boxed/struct.Box.html)】，C++也可以用OOP方式编写。
- body_parser用于处理multi_part文件上传，不单支持单文件支持自定义大小，甚至还可以支持多文件上传并结合了mmap的零拷贝的存储映射。
- lexical_cast用于从字符串，字符视图到基本类型之间的互相转换，从0-1实现的是将header-only变为头文件源文件分离，并加速了编译速度。
- router采用了动态路由技术，可以增删改查，目前还支持使用正则表达式，这是其他框架恰恰不具备的。
- text是从0-1用于解决utf8的字符串存储，并且可以自行设定容量，每个容量都是针对utf8的，可以应对数据库中的varchar类型。
- timer是从0-1用于解决c++没有javascript计时器的问题的，并且才用了头文件源文件分离，加速了编译速度。
- 以上部件是个人技术实力的展示。当然，也有一些部件，其中并不完全是从0到1，但那些不是最重要的部件。

## 特征
- 基于epoll架构的全平台支持[windows下由wepoll实现]
- 现在最低编译器版本支持到了c++11, 目前兼容了c++17的许多特性包括any, optional, string_view, 以及部分的扩展
- 最少的第三方库，均以源文件形式存放项目中
- 难以置信的编译速度，开发速度同时也得到提升
- 支持网页版Postman，地址是127.0.0.1:8080/test.html
- 可以对路由进行增删改查，动态路由参考【[lithium](https://github.com/matt-42/lithium)】迭代而成
- 支持类似于无栈协程的非对称协程, 像python的生成器, 协程来自【[boost](https://github.com/boostorg/context)】
- 拥有类似nodejs的api，写起来也像js, http解析器来自【[h2o](https://github.com/h2o/picohttpparser)】【[nodejs](https://github.com/nodejs/llhttp)】
- 字符串映射的str_map【[beast](https://github.com/boostorg/beast),[crow](https://github.com/ipkn/crow)】
- 静态文件缓存file_sptr【[libhttp](https://github.com/neithern/libhttp)】
- 支持单元测试，文档请见【[coost](https://coostdocs.gitee.io/cn/co/unitest/)】
- Json集成于【[coost](https://coostdocs.gitee.io/cn/co/json/)】, 并支持序列化与反序列化
- zlib压缩来自【[zlib](https://github.com/madler/zlib)】
- mman来自【[mman](https://code.google.com/archive/p/mman-win32/source/default/source)】
- cache_file来自【[drogon](https://github.com/drogonframework/drogon/blob/master/lib/src/CacheFile.cc)】

## 仍在开发中
- [x] 动态路由
- [x] gzip压缩
- [x] body-parser的支持
- [x] ssl证书
- [ ] websocket
- [x] 协程(c++11 ~ c++20)
- [ ] udp服务端
- [x] tcp客户端

## 说明
- 协程的原理图
- ![yield](./yield.gif)
- [示例](http://165.154.225.219:8080/)🚀
- ![测试](./test.jpg)

## 例子
```c++
using namespace fc;
int main() {
  App app; app fc_app(web) fc_app(user);
  app.file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4","webm","mp3","wav","aac" })
    .sub_api("/", app.serve_file("static")).set_keep_alive(4, 3, 2).set_use_max_mem(600.0).set_file_download(true);
  app.default_route() = [](Req& req, Res& res)_ctx {
    res.set_content_type("text/html;charset=UTF-8", 23); res.set_status(404);
    res.write_async_s([] {
      char name[64]; gethostname(name, 64); Json x{ {"header", name} }; return mustache::load("404NotFound.html").render(x);
      }); co_return;//设置默认的路由
  };
  app["/redirect"] = [](Req& req, Res& res)_ctx {
    res.redirect("https://www.github.com"); co_return;
  };
  app["/get_upload"] = [](Req& req, Res& res)_ctx {
    res.write_async([] {
      auto f = fc::directory_iterator(fc::directory_ + fc::upload_path_); Json x;
      std::set<std::string_view> extentions = { "mp4", "mp3", "webm", "wav", "mkv" };
      for (auto v : f) {
        if (std::find(extentions.begin(), extentions.end(), fc::toLowerCase(v.ext)) != extentions.end()) {
          x.push_back({ {"name",v.name.substr(fc::directory_.size())}, {"size",v.size} });
        }
      } return x;
      }); co_return;//获取上传的文件列表
  };
  app["/read_file"] = [](Req& req, Res& res)_ctx { res.write_async([] { Json x = json::read_file("test.json"); return x; }); co_return; };
  app["/json"] = [](Req& req, Res& res)_ctx {
    Json x; Book b{ "ts", Person{"js",6, Book{"plus" }, vec<Book>{ {"1", Person {"sb" }}, {"2", Person {"sb" }} }} };
    b.person->book = Book{ "rs", null, vec<Person>{ {"?"}, {"!"} } }; x = b; res.write(x); co_return;//json请求
  };
  app["/serialization"] = [](Req& req, Res& res)_ctx {
    Json x = json::parse(R"(
    {"name":"ts","person":{"name":"js","age":33,"book":{"name":"ojbk","person":{"name":"fucker","age":0},
    "persons":[{"name":"stupid","age":1},{"name":"idoit","age":2},{"name":"bonkers","age":3,"book":{"name":"sb"}}]}}}
    )"); Book b = x.get<Book>(); b.person->book->persons[2].name = "wwzzgg"; x = b; res.write(x.dump()); co_return;//反序列化与序列化
  };
  app["/api"] = [](Req& req, Res& res)_ctx { res.write(res.app._print_routes()); co_return; };//返回路由列表
  app.post("/api") = [](Req& req, Res& res)_ctx {
    BP bp(req, 1000); co_await bp.run(); std::string s;//支持上传的文件总大小1000MB
    for (auto p : bp.params) {
      s << (p.key + ": ") << p.value << ", ";
    }
    s.pop_back(); s.pop_back(); res.write(s); co_return;
  };
  app["/del"] = [](Req&, Res& res)_ctx { res.app["/"] = nullptr; res.write("主页的路由已被删除！！"); co_return; };
  app["/timer"] = [](Req& req, Res& res)_ctx {
    req.setTimeout([] { raise(SIGINT); }, 6000); res.write("关闭服务倒计时启动！"); co_return;
  };
  //启动服务器，同样支持ipv6
  app.http_serve(8080);
}
```

### 构建（测试、示例）
建议使用CMake进行源代码外构建。
如果构建失败，请删除清理cmake缓存, 建议先删除build目录。
`cmake -B build`后的额外编译选项。
使用vcpkg `-DCMAKE_TOOLCHAIN_FILE=../vcpkg.cmake`
使用llhttp解析器 `-DLLHTTP=1`
使用openssl `-DOPENSSL=1`
```
cmake -B build -DLLHTTP=0 -DOPENSSL=0
cmake --build ./build --config Release -j
```
