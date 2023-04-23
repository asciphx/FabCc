# FabCc(1.4-RTM)
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
Inspired by other well-known C++ web frameworks, FabCc's positioning is a network framework, which is characterized by low code, high performance, strong type, super standard, safest and awesome.
## Eng | [简中](./README-zh_CN.md)
> The released version is the stable version, and the branch may not compile successfully on certain platforms because the branch is a development version.
## Features
- Full platform support based on epoll architecture [implemented by wepoll under windows]
- Now the minimum compiler supports the c++14 version, and is currently compatible with many features of C++17 including any, optional, string_view, and some extensions
- The fewest third-party libraries, are stored in the project in the form of source files
- Fastest API, such as lexical_cast, EncodeURL, DecodeURL, itoa
- Incredible compilation speed and development speed are also improved
- Support the web version of Postman, the address is 127.0.0.1:8080/test.html
- Can add, delete, modify and query the route. Dynamic Route Reference[[lithium](https://github.com/matt-42/lithium)]Iterative
- Supports asymmetric coroutines similar to stackless coroutines, such as Python's generator. Coroutine trip from[[boost](https://github.com/boostorg/context)]
- With an API similar to nodejs, it also looks like JS, llhttp parser from[[nodejs](https://github.com/nodejs/llhttp)]
- str_map for string mapping taken from[[beast](https://github.com/boostorg/beast)],[[crow](https://github.com/ipkn/crow)]
- file_sptr for Static file cache from[[libhttp](https://github.com/neithern/libhttp)]
- Support unit testing. Please refer to [[coost](https://coostdocs.github.io/cn/co/unitest/)] for documentation
- Json integrated in[[coost](https://coostdocs.github.io/cn/co/json/)], and supports serialization and deserialization
- box with design style from the Rust language[[std::boxed::Box](https://doc.rust-lang.org/std/boxed/struct.Box.html)],   
  so C++ can also be written in the same OOP way as Java without using raw pointers.
- Zlib compression from[[zlib](https://github.com/madler/zlib)]
- mman from [[mman](https://code.google.com/archive/p/mman-win32/source/default/source)]
- cache_file from [[drogon](https://github.com/drogonframework/drogon/blob/master/lib/src/CacheFile.h)]

## Still under development
- [x] Processing of routing brace
- [x] Gzip compression
- [x] Body parser
- [ ] SSL certificate support
- [ ] WebSocket
- [x] Coroutines
- [ ] UDP server
- [ ] TCP client

## Description
- If translated by machine, FabCc can be a wafer factory, so FabCc symbolizes the most sophisticated and minimal chip.
- The namespace uses the uppercase hump fc of FabCc.
- Principle of Coroutines
- ![yield](./yield.gif)
- [Demo site](http://8.129.58.72:8080/)🚀
- ![test](./test.jpg)

## example
```c++
using namespace fc;
void funk(Req& req, Res& res) {
  res.write("Homepage route is replicated by std::bind！");
};
int main() {
  App app; Timer t;
  app.file_type({ "html","htm","ico","css","js","json","svg","png","jpg","gif","txt","wasm","mp4" })
    .sub_api("/", app.serve_file("static"));//Service file interface
  app["/json"] = [](Req& req, Res& res) {
    Book b{ "ts", Person{"js",33, Book{ "plus" }, vec<Book>{ Book{"1", Person { "sb1" }},Book{"2", Person { "sb2" }} }} };
    b.person->book = Book{ "rs" };//Write C++ like Object-Oriented Programming
    Json x; to_json(x, &b); x["person"]["book"]["person"] = b.person; res.write(x.dump());
  };
  app["/serialization"] = [](Req& req, Res& res) {
    Json x; Book b; from_json(x = json::parse(R"(
	{"name":"ts","person":{"name":"js","age":33,"book":{"name":"ojbk","person":{"name":"fucker","age":0},
	"persons":[{"name":"stupid","age":1},{"name":"idoit","age":2},{"name":"bonkers","age":3,"book":{"name":"sb"}}]}}}
	)"), &b); b.person->book->persons[2].name = "wwzzgg"; to_json(x, &b); res.write(x.dump());
  };
  app["/api"] = [&app](Req& req, Res& res) {
    res.write(app._print_routes());//Return to routing list
  };
  app.post("/api") = [](Req& req, Res& res) {
    BP bp(req, 1000);//Support for uploading files with a total size of 1000MB
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
    res.write("Turn off the server timer and start the countdown！");
    app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //Start the server
  http_serve(app, 8080);
}
```

### Building (Tests, Examples)
Out-of-source build with CMake is recommended.
delete clean cmake cache if build fails.
```
mkdir build
cd build
cmake ..
cmake --build ./ --config Release
```
