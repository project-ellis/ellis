#undef NDEBUG
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <iostream>
#include <stdlib.h>


int main(int argc, char *argv[]) {

  using std::cerr;
  using std::endl;

  if (argc != 3) {
    cerr << "Syntax: " << argv[0] << " in_filename out_filename" << endl;
    exit(1);
  }

  const char *in_filename = argv[1];
  const char *out_filename = argv[2];

  try {
    auto n = ellis::load_file_autodecode(in_filename);
    ellis::dump_file_autoencode(n.get(), out_filename);
  }
  catch (const ellis::err &e) {
    cerr << "ERROR: " << e.msg() << endl << "Details\n" << e.summary() << endl;
    exit(1);
  }

  return 0;
}
