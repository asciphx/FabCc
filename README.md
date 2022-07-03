# FabCc
Concise, fast, practical, reactive, functional.

## fast_lexical_cast
- Because template functions cannot be inlined on g++, they can only be used in the way of HPP pure header files.
- The disadvantage is that the compilation speed is reduced by about 8%.
- The advantage is that inlining brings 50% performance improvement (in g++ compiler).
- ![fast_lexical_cast](./fast_lexical_cast.jpg)
