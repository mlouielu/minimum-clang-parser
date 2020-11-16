#include <armadillo>

class Foo {
public:
  // arma::vec foobar;
  int bar;
  arma_aligned const double *const foobarfoo;
  int token;
};

int main() { return sizeof(Foo); }
