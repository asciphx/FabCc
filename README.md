# FabCc(v1.0-UHD)
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
Inspired by other well-known C++ web frameworks, FabCc's positioning is a network framework, which is characterized by low code, high performance, strong type, super standard, safest and awesome. The logo uses Nod's logo, and the design comes from Command and Conquer(CNC).

![FabCc](./static/logo.png)
## Eng | [简中](./README-zh_CN.md)
> On April 12th, the ultra high definition 8k remastered version arrived. Support Gzip compression of web pages to reduce traffic consumption. Fixed various bugs, compatible with modern JSON and C++11, fixed keep alive mechanism, and launched best C++20 stack less coroutine. The following is a comparison chart.
> ![coroutine](./co%20vs%20Task.jpg)

## Be Original
- Fully and deeply integrated RBTree and time wheel have stronger performance (+5%) encapsulated revocable timers, using box<> to manage the release of coroutines, thus saving the memory of using containers
- Supports c++20 stack free coroutines, currently compatible with the original project's stack based asymmetric coroutines, and is perfectly integrated, requiring almost minimal macro modifications. Performance is about 5% stronger than stack based protocols.
- Added a new player with subtitles [The subtitles have the same name as the file, but the file format is different, that is, the suffix is different (supports srt, vtt, ass formats)] func.
- Enhanced field reflection, for example`std::string_view sv = k(&O::id);` will return "`O`.`id`"(can be modified by constexpr in C++14 and higher versions).
- The tcp client based on openssl has preliminary support and limited functions, so most tests can pass.
- Support jsonc format for compatible annotations. However, this is only called when reading the json file.
- The syntax of modern json, its full compatibility with Almost all stl containers, and its full range of macros are used to serialize and deserialize at Compile time.
- Support multi-core compilation options on MSVC, use `cmake --build ./build --config Release -j` to compile in parallel.
- Supports 8K ultra-high-definition video playback, no freeze, no delay, and unlocks the playback of videos larger than 4GB (highlight).
- Support Gzip compression function, the default cache is 6 seconds, you can modify it in the CACHE_HTML_TIME_SECOND macro configuration. When the compression ratio is 6, so I personally think it is the optimal solution for efficiency and compression ratio.
- The whole platform supports range requests in video or audio format, and you can jump to any playback point on demand. For example wav, mp3, mp4, webm. And you can set whether to allow downloading, and then also support pausing or continuing downloading.
- box with design style from the Rust language[[std::boxed::Box](https://doc.rust-lang.org/std/boxed/struct.Box.html)],   
  so there is no need to use raw pointers, and C++can also be written in OOP mode.
- Derived from box and solved the problem of circular dependency from 0-1 perfectly, and super smart pointer box can replace optional, compatible with std:: optional.
- The current box can also replace the original pointer, automatically managing memory from 0 to 1 without considering memory release, equivalent to automatic GC, aimed at scripting c++.
- body_parser for handling multi_ Part file upload not only supports single file support for custom size, but also supports multiple file uploads and zero copy storage mapping combined with mmap.
- lexical_cast is used to convert strings, character views, and basic types to each other. From 0 to 1, it achieves the separation of header only to header file source files and accelerates compilation speed.
- Router adopts dynamic routing technology, which can be added, deleted, modified, and checked. Currently, it also supports the use of regular expressions, which is precisely not available in other frameworks.
- string_view is used to supplement the inability to use string in C++14 and older versions_ The pain points of view and the maximum expansion of features in the new version.
- text is used to store strings from 0 to 1 for utf8, and the capacity can be set independently. Each capacity is specific to utf8 and can handle varchar types in the database.
- timer is used from 0 to 1 to solve the problem of C++without JavaScript timers, and only uses header file source file separation to accelerate compilation speed.
- The above original components are a demonstration of personal technical strength. Of course, there are also some components, which are not entirely from 0 to 1, but those are not the most important components.

## Features
- The front-end player only uses Xigua player, and has a new subtitle func. Xigua comes from [[xgplayer](https://github.com/bytedance/xgplayer)]
- Full platform support based on epoll architecture [implemented by wepoll under windows]
- Now the minimum compiler supports the c++11 version, and is currently compatible with many features of C++17 including any, optional, string_view, and some extensions
- The fewest third-party libraries, are stored in the project in the form of source files
- Fastest API, such as lexical_cast, EncodeURL, DecodeURL, itoa
- Incredible compilation speed and development speed are also improved
- Support the web version of Postman, the address is 127.0.0.1:8080/test.html
- Can add, delete, modify and query the route. Dynamic Route Reference[[lithium](https://github.com/matt-42/lithium)]Iterative
- Supports asymmetric coroutines similar to stackless coroutines, such as Python's generator. Coroutine trip from[[boost](https://github.com/boostorg/context)]
- With an API similar to nodejs, it also looks like JS, llhttp parser from[[h2o](https://github.com/h2o/picohttpparser)] [[nodejs](https://github.com/nodejs/llhttp)]
- str_map for string mapping taken from[[beast](https://github.com/boostorg/beast)],[[crow](https://github.com/ipkn/crow)]
- file_sptr for Static file cache from[[libhttp](https://github.com/neithern/libhttp)]
- Support unit testing. Please refer to [[coost](https://coostdocs.github.io/cn/co/unitest/)] for documentation
- Json integrated in[[coost](https://coostdocs.github.io/cn/co/json/)], and supports serialization and deserialization
- Zlib compression from[[zlib](https://github.com/madler/zlib)]
- mman from [[mman](https://code.google.com/archive/p/mman-win32/source/default/source)]
- cache_file from [[drogon](https://github.com/drogonframework/drogon/blob/master/lib/src/CacheFile.h)]

## Still under development
- [x] Dynamic routing
- [x] Gzip compression
- [x] Body parser
- [x] SSL certificate support
- [ ] WebSocket
- [x] Coroutines(c++11 ~ c++20)
- [ ] UDP server
- [x] TCP client

## Description
- Principle of Coroutines
- ![yield](./yield.gif)
- [Demo site](http://8.129.58.72:8080/)🚀
- ![test](./test.jpg)

## example
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
      }); co_return;//Set default route
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
      }); co_return;//Get the list of uploaded files
  };
  app["/read_file"] = [](Req& req, Res& res)_ctx { res.write_async([] { Json x = json::read_file("test.json"); return x; }); co_return; };
  app["/json"] = [](Req& req, Res& res)_ctx {
    Json x; Book b{ "ts", Person{"js",6, Book{"plus" }, vec<Book>{ {"1", Person {"sb" }}, {"2", Person {"sb" }} }} };
    b.person->book = Book{ "rs", null, vec<Person>{ {"?"}, {"!"} } }; x = b; res.write(x); co_return;//Return json
  };
  app["/serialization"] = [](Req& req, Res& res)_ctx {
    Json x = json::parse(R"(
    {"name":"ts","person":{"name":"js","age":33,"book":{"name":"ojbk","person":{"name":"fucker","age":0},
    "persons":[{"name":"stupid","age":1},{"name":"idoit","age":2},{"name":"bonkers","age":3,"book":{"name":"sb"}}]}}}
    )"); Book b = x.get<Book>(); b.person->book->persons[2].name = "wwzzgg"; x = b; res.write(x.dump()); co_return;//Deserialization and serialization
  };
  app["/api"] = [](Req& req, Res& res)_ctx { res.write(res.app._print_routes()); co_return; };//Return to routing list
  app.post("/api") = [](Req& req, Res& res)_ctx {
    BP bp(req, 1000); co_await bp.run(); std::string s;//Support for uploading files with a total size of 1000MB
    for (auto p : bp.params) {
      s << (p.key + ": ") << p.value << ", ";
    }
    s.pop_back(); s.pop_back(); res.write(s); co_return;
  };
  app["/del"] = [](Req&, Res& res)_ctx { res.app["/"] = nullptr; res.write("The routing of the home page is delete！！"); co_return; };
  app["/timer"] = [](Req& req, Res& res)_ctx {
    req.setTimeout([] { raise(SIGINT); }, 6000); res.write("Turn off the server timer and start the countdown！"); co_return;
  };
  //Start the server, also supports ipv6
  app.http_serve(8080);
}
```

### Building (Tests, Examples)
Out-of-source build with CMake is recommended.
delete clean cmake cache if build fails.
`cmake -B build` followed by additional compilation options.
Use vcpkg `-DCMAKE_TOOLCHAIN_FILE=../vcpkg.cmake`
Use llhttp parser `-DLLHTTP=1`
Use openssl `-DOPENSSL=1`
```
cmake -B build -DLLHTTP=0 -DOPENSSL=0
cmake --build ./build --config Release -j
```
