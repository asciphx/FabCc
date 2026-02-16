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
