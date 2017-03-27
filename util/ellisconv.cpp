/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


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
