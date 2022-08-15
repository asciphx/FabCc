#include "unitest.hh"

int main(int argc, char** argv) {
  edge::init(argc, argv);
  unitest::run_all_tests();
  return 0;
}
