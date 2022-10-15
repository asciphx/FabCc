# FabCc
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
Inspired by other well-known C++ web frameworks, FabCc's positioning is a network framework, which is characterized by concise, fast, practical, reactive, functional.
## Features
- Full platform support based on epoll architecture [implemented by wepoll under windows]
- Now the minimum compiler supports the c++14 version, and is currently compatible with many features of C++17 including any, optional, string_view, and some extensions
- The fewest third-party libraries, are stored in the project in the form of source files
- Fastest API, such as lexical_cast, EncodeURL, DecodeURL
- Incredible compilation speed and development speed are also improved
- Support the web version of Postman, the address is 127.0.0.1:8080/test.html
- Can add, delete, modify and query the route. Dynamic Route Reference[[lithium](https://github.com/matt-42/lithium)]Iterative
- Supports asymmetric coroutines similar to stackless coroutines, such as Python's generator. Coroutine trip from[[boost](https://github.com/boostorg/context)]
- With an API similar to nodejs, it also looks like JS, llhttp parser from[[nodejs](https://github.com/nodejs/llhttp)]
- str_map for string mapping taken from[[beast](https://github.com/boostorg/beast)],[[crow](https://github.com/ipkn/crow)]
- file_sptr for Static file cache from[[libhttp](https://github.com/neithern/libhttp)]
- Support unit testing. Please refer to [[coost](https://coostdocs.github.io/cn/co/unitest/)] for documentation
- Json integrated in[[coost](https://coostdocs.github.io/cn/co/json/)], and supports serialization and deserialization
- box with design style from the Rust language[[std::boxed::Box](https://doc.rust-lang.org/std/boxed/struct.Box.html)]，so there is no need to use naked pointers
- Zlib compression from[[zlib](https://github.com/madler/zlib)]

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
  Timer t; App app; Tcp srv;
  app.sub_api("/", app.serve_file("static"));//Service file interface
  app["/json"] = [](Req& req, Res& res) {
	Json x = { { "h", 23 }, { "b", false }, { "s", "xx" }, { "v", {1,2,3} }, { "o", {{"xx", 0}} } };
	res.write(x.dump());//JSON response
  };
  app["/sockets"] = [&srv](Req& req, Res& res) {
	Buf b("("); b << srv.$.size() << ")[";
	for (std::set<u32>::iterator i = srv.$.begin(); i != srv.$.end(); ++i) b << *i << ',', Conn::shut(*i, _READ);
	res.write(b.pop_back() << ']');//Get all active socket id, and close all sockets
  };
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//Return to routing list
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
  };//co's yield function is used to ensure the execution order
  app["/del"] = [&app](Req&, Res& res) {
	app.get() = nullptr;
	res.write("The routing of the home page is delete！！");
  };
  app["/timer"] = [&](Req&, Res& res) {
	t.setTimeout([&srv] {
	  printf("The route has been idle for 1 minute, and the server will shut down automatically！！");
	  srv.exit();
	}, 60000);
	res.write("Turn off the server timer and start the countdown！");
	app.get() = std::bind(funk, std::placeholders::_1, std::placeholders::_2);
  };
  //Start the server
  srv.router(app).timeout(6000).setTcpNoDelay(true).setThread().Start("0.0.0.0", 8080);
  return 0;
}
```

### Building (Tests, Examples)
Out-of-source build with CMake is recommended.
```
mkdir build
cd build
cmake ..
cmake --build ./ --config Release
```
