# TODO

## Initial release to public

M Msgpack encoder
* Name registration for encode/decode
* Run doxygen for purposes of checking inline docs.
* Doxygen consistency, including using comments from the C headers if/when
  appropriate.
* Pick license.
* General documentation for public.
* Build instructions.
* Github workflow instructions.
* (jmc) Write unit test that catches u8str deep copy bug.
* Codec framework layer that handles framing and simple issues like forgetting
  to call reset at the right time.

## Maturation

* Finish base64 encoded binary mode for JSON.
* Cython wrappers or SWIG.
* Cross-build for at least ARM.
* C wrappers.
* Sample application code.
* Sample codec (delimited text codec--improve).
* CSV codec
* PNM codec
* Command line converter and pipeline tool.
* Codec registration system via shared libraries, dlopen, etc.
* Schema archetypal happy case test code.
* Schema API
* Schema C++ code
* Schema tests
* Schema wrappers
* XML codec
* YAML codec
* HDF codec
* ROS codec
* Zero-copy I/O for `binary_node`?
* Make it easier to install ninja
* Write at least one simple perf test (use meson benchmark?)
* Perf comparison of `ELLIS_ASSERT_EQ` vs `ELLIS_ASSERT_OP`, which calls the
  copy-constructor.
* Add some form of protection against really deep documents (also, define
  "deep").
* Do a perf run to see if there are any major blunders.
* Investigate an automatic but predictable perf test.
* Extensive tests, including fuzz tests, with valgrind.

## Future improvements

* Make clang-format support our coding style programatically.
* Meson understands the enabling of different codecs (affects lib, headers).
* Issue tracking, documented in README.
* Public mailing list/group/whatever?
* Man pages.
* Packaging for debian.
* Packaging for redhat.
