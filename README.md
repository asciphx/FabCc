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

## Features
- Full platform support based on epoll architecture [implemented by wepoll under windows]
- Now the minimum compiler supports the c++11 version, and is currently compatible with many features of C++17 including any, optional, string_view, and some extensions
- The fewest third-party libraries, are stored in the project in the form of source files
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
- [Demo site](http://165.154.225.219:8080/)🚀
- ![test](./test.jpg)

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
