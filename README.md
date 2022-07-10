# FabCc
Concise, fast, practical, reactive, functional. Inspired by other well-known c++ web frameworks.
## Description
- The namespace uses the uppercase hump fc of FabCc.
- For the time being, it only provides the server development of Windows version based on libuv, and plans to support Linux and MAC in the future.
- -For the time being, the buffer needs to be buffered, so you have to refresh the page several times to warm up before testing. Therefore, you need to transform the thread pool, but it will be more complex.
- ![single thread test](./single_core_test.jpg)
## fast_lexical_cast
- Template functions cannot be inlined on g++ if they are not '.hpp' files
- The disadvantage is to sacrifice performance in exchange for compilation speed
- The advantage is that the development speed has been greatly improved
- ![fast_lexical_cast](./fast_lexical_cast.jpg)
