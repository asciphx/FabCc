# FabCc
[![license][license-badge]][license-link]
![platform][supported-platforms-badge]
[![release][release-badge]][release-link]

[license-badge]: https://img.shields.io/badge/License-AGPL%20v3-gold.svg
[license-link]: LICENSE
[supported-platforms-badge]: https://img.shields.io/badge/platform-Win32%20|%20GNU/Linux%20|%20macOS%20|%20FreeBSD%20-maroon
[release-badge]: https://img.shields.io/github/release/asciphx/FabCc.svg?style=flat-square
[release-link]: https://github.com/asciphx/FabCc/releases
Concise, fast, practical, reactive, functional. Inspired by other well-known C++ web frameworks, which can be called the C++ version of nodejs.

## Features
- Can add, delete, modify and query the route
- Use the global timer task to control some things, such as shutting down the server when it expires
- With an API similar to nodejs, it also looks like JS
- Minimalist API, infinite possibilities
- Fastest API, such as lexical_cast, EncodeURL, DecodeURL
- Incredible compilation speed and development speed are also improved
- The fewest third-party libraries, are stored in the project in the form of source files
- Cross platform support, (Linux and windows have been tested)
- Minimize the allocate and release of memory, which is friendly to the hard disk and does not generate a large amount of memory fragments, so there is almost no disk IO
- Support unit testing. Please refer to [[coost](https://coostdocs.github.io/cn/co/unitest/)] for documentation
- Support the web version of Postman, the address is 127.0.0.1:8080/test.html
- Json integrated in【[coost](https://coostdocs.github.io/cn/co/json/)】, and supports serialization and deserialization
- box with design style from the rust language, so there is no need to use naked pointers

## Still under development
- [x] Processing of routing brace
- [x] Gzip compression
- [x] Body parser
- [ ] SSL certificate support
- [ ] WebSocket
- [ ] UDP server
- [ ] TCP client

## Description
- If translated by machine, FabCc can be a wafer factory, so FabCc symbolizes the most sophisticated and minimal chip.
- The namespace uses the uppercase hump fc of FabCc.
- The logo hasn't been replaced yet. It's the crow logo for the time being.
- Regex expression routing, because performance issues will be removed.
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
  app["/api"] = [&app](Req& req, Res& res) {
	res.write(app._print_routes());//Return to routing list
  };
  app.post("/api") = [](Req& req, Res& res) {
	BP bp(req, 4096);
	for (auto p : bp.params) {
	  res.write(p.key + ": " + (!p.size ? p.value : p.filename) + ", ");
	}
  };
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
  srv.router(app).timeout(6000).setTcpNoDelay(true).Start("0.0.0.0", 8080);
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
