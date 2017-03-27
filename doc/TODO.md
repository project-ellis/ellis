# TODO

## Initial TODOs

* (jmc) Make delimited text codec more useful.
* (jmc) Make a more proper `delimited_text_test` from the `test/codec_test`.
* (both) Doxygen consistency

## Maturation

* Run doxygen for purposes of checking inline docs.
* Codec framework layer that handles framing and simple issues like forgetting
  to call reset at the right time.
* Make the msgpack encoder not inefficiently copy its input by parsing
  per-character instead of per-node and representing all state explicitly via
  stacks.
* Various TODO items in the msgpack decoder (noted in comments).
* Cleanup the ELM327 and OBD contracts to make sure they are clear, correct, and
  consistent about CONTINUE behavior.
* Finish base64 encoded binary mode for JSON.
* Cython wrappers or SWIG.
* Cross-build for at least ARM.
* C wrappers.
* Sample application code.
* Sample codec (delimited text codec--improve).
* Command line converter and pipeline tool.
* Codec registration system via shared libraries, dlopen, etc.
* Schema archetypal happy case test code.
* Schema API
* Schema C++ code
* Schema tests
* Schema wrappers
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

## Codecs

* CSV codec
* PNM codec
* ORC codec
* Parquet codec
* XML codec
* YAML codec
* HDF codec
* ROS codec

## Future improvements

* Make clang-format support our coding style programatically.
* Meson understands the enabling of different codecs (affects lib, headers).
* Issue tracking, documented in README.
* Public mailing list/group/whatever?
* Man pages.
* Packaging for debian.
* Packaging for redhat.

## Sample community tasks

Lots of ellis cleanup work to do.  Can make some of it be contributable by
community (e.g. testing, new codecs).

* more codecs
* more transformations
* project ellis icon--melting pot? island?
* manual page
* tldr page
* packaging for various distros
* benchmarks
* more correctness and fuzz tests
* code review
