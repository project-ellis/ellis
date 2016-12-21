# TODO

## Initial release to public

M Msgpack codec
* Name registration for encode/decode
* Run doxygen for purposes of checking inline docs.
* Doxygen consistency, including using comments from the C headers if/when
  appropriate.
* Pick license.
* General documentation for public.
* Build instructions.
* Github workflow instructions.

## Maturation

* Sample code.
* Sample encoders/decoders.
* Support half a dozen formats that people want.
* Command line converter tool.
* Codec registration system via shared libraries, dlopen, etc.
* Cython wrappers or SWIG.
* Cross-build for at least ARM.
* C wrappers.
* Schema archetypal happy case test code.
* Schema API
* Schema C++ code
* Schema C wrappers
* Schema tests
* Zero-copy I/O for `binary_node`?
* Make it easier to install ninja
* Create a `u8str_node`.
* Write at least one simple perf test.
* Perf comparison of `ELLIS_ASSERT_EQ` vs `ELLIS_ASSERT_OP`, which calls the
  copy-constructor.
* Add some form of protection against really deep documents (also, define
  "deep").
* Do a perf run to see if there are any major blunders.
* Investigate an automatic but predictable perf test.
* XML codec
* YAML codec
* ROS codec?
* Extensive tests, including fuzz tests, with valgrind.

## Future improvements

* Make clang-format support our coding style programatically.
* Meson understands the enabling of different codecs (effects lib, headers).
* Issue tracking, documented in README.
* Man pages.
* Packaging for debian.
* Packaging for redhat.
