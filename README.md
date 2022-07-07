# FabCc
Concise, fast, practical, reactive, functional.
## Description
- The namespace uses the uppercase hump FC of fabcc.
- For the time being, it only provides the server development of Windows version based on libuv, and plans to support Linux and MAC in the future.
## fast_lexical_cast
- Template functions cannot be inlined on g++ if they are not '.hpp' files
- The disadvantage is to sacrifice performance in exchange for compilation speed
- The advantage is that the development speed has been greatly improved
- ![fast_lexical_cast](./fast_lexical_cast.jpg)
