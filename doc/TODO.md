# TODO

## Getting system off the ground--basic flight test

J:Schema archetypal happy case test code.
B:Schema API
* Schema C++ code
* Schema C wrappers
* Schema tests
J:array-node correctness tests.
M:C wrappers.
J:Zero copy I/O interface.
J:Check whether C++ std::allocator types can form backbone of allocators.
M:Decoders and encoder C++ API.
J:JSON encoder/decoder.
M:OBD decoder.

## Initial sanity cleanup

* Doxygen consistency.
* Moving of comments from C to C++.

## Probably need real soon

J:Simple arena tests.
J:Cython wrappers or SWIG.
M:What does meson's static analysis etc do?  Arrange for cppcheck?
M:Cross-build for at least ARM.

## Maturation

M Make it easier to install ninja
* Decide whether to keep std::string.
* Write at least one simple perf test.
* Do a perf run to see if there are any major blunders.
* Investigate an automatic but predictable perf test.
* Msgpack encoder/decoder.
* XML encoder/decoder.
* YAML encoder/decoder.

## More cleanup

* Compilation test with clang.
* Turn on code coverage.
* Run doxygen for purposes of checking inline docs.
* Extensive tests, including fuzz tests, with valgrind.

## Releasing to public

* Pick license.
* General documentation for public.
* Build instructions.
* Github workflow instructions.
* Sample code.
* Sample encoders/decoders.
