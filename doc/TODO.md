# TODO

## Getting system off the ground--basic flight test

J:Schema archetypal happy case test code.
B:Schema API
* Schema C++ code
* Schema C wrappers
* Schema tests
* Zero-copy I/O for `binary_node`?
M:C wrappers.
J:JSON encoder/decoder.
M:OBD-over-CAN decoder.

## Initial sanity cleanup

* Doxygen consistency.
* Moving of comments from C to C++.

## Probably need real soon

J:Cython wrappers or SWIG.
M:Cross-build for at least ARM.

## Maturation

M Make it easier to install ninja
* Create a `u8str_node`.
* Write at least one simple perf test.
* Do a perf run to see if there are any major blunders.
* Investigate an automatic but predictable perf test.
* Msgpack codec
* XML codec
* YAML codec
* ROS codec?

## More cleanup

* Run doxygen for purposes of checking inline docs.
* Extensive tests, including fuzz tests, with valgrind.

## Initial release to public

* Command line converter tool.
* Support half a dozen formats that people want.
* Pick license.
* General documentation for public.
* Build instructions.
* Github workflow instructions.
* Sample code.
* Sample encoders/decoders.

## Further improvements

* Make clang-format support our coding style programatically.
* Meson understands the enabling of different codecs (effects lib, headers).
* Codec registration system via shared libraries, dlopen, etc.
* Issue tracking, documented in README.
* Manual pages.
* Packaging for debian.
* Packaging for redhat.
