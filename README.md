Minimum Clang Parser
====================

This parser is used to test about why using `libclang` to parse C++ source code,
causing `__attribute__` disappear in `nasa/trick` Interface Code Generator (ICG).

This parser give the skelton of how `trick-ICG` works and will dump out the
AST by the Clang AST consumer `ASTPrinter`.


Why
---

I found that `trick-ICG` will give out wrong memory layout of the `arma::vec`, and
relative stuff. Clang AST dumping & armadillo source code shows that `arma::vec` layout
looks like this, having `__attribute__((__aligned__))` for `mem` pointer:

```
$ clang -Xclang -ast-dump -fsyntax-only test.cpp
...

```


When reproduce by in signle file w/o include, `trick-ICG` and the `parser.cpp` have the
same result as `clang`, which will calculate the offset with correct padding.



TBA
---
