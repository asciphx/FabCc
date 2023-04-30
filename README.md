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

## Be Original
- box with design style from the Rust language[[std::boxed::Box](https://doc.rust-lang.org/std/boxed/struct.Box.html)],   
  so there is no need to use raw pointers, and C++can also be written in OOP mode.
- Derived from box and solved the problem of circular dependency from 0-1 perfectly, and super smart pointer box can replace optional, compatible with std:: optional
- The current box can also replace the original pointer, automatically managing memory from 0 to 1 without considering memory release, equivalent to automatic GC, aimed at scripting c++
- body_ Parser for handling multi_ Part file upload not only supports single file support for custom size, but also supports multiple file uploads and zero copy storage mapping combined with mmap
- lexical_ Cast is used to convert strings, character views, and basic types to each other. From 0 to 1, it achieves the separation of header only to header file source files and accelerates compilation speed
- Router adopts dynamic routing technology, which can be added, deleted, modified, and checked. Currently, it also supports the use of regular expressions, which is precisely not available in other frameworks
- Designed a Buf class to replace strings at certain times, and also provided a reset method for reusing memory instead of being inefficient like clear
- string_ View is used to supplement the inability to use string in C++14 and older versions_ The pain points of view and the maximum expansion of features in the new version
- text is used to store strings from 0 to 1 for utf8, and the capacity can be set independently. Each capacity is specific to utf8 and can handle varchar types in the database
- timer is used from 0 to 1 to solve the problem of C++without JavaScript timers, and only uses header file source file separation to accelerate compilation speed
- The above original components are a demonstration of personal technical strength. Of course, there are also some components, which are not entirely from 0 to 1, but those are not the most important components.

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
    .sub_api("/", app.serve_file("static")).set_keep_alive(4, 3, 2).set_use_max_mem(300.0);//Service file interface
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
  app.http_serve(8080);
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
