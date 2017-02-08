# Ellis

Everybody has a favorite general-purpose data format.  Json, Msgpack, Yaml,
ORC, Parquet, and so forth.  If you like something else, you must convert.

This is how it was with images, once upon a time.  The people grew tired of
this treadmill, instead adopting smart input libraries that support all of
them and just give you a neutral in-memory representation.

Ellis does that for general data, and it gives you some convenient tools
for interoperability.

At heart, ellis seeks to provide cross-language bindings for a common
in-memory representation of hierarchical data, along with encoder and decoder
plugins for common data formats, schemas, and streaming mechanisms.

#### Advantages to this approach

- *Simplicity*: Many libraries require some particular data format
  which may only be available in some particular programming language. If
  you want to use a different format, you need a different library.  If you
  want to use a different language, you again need to find a different
  library, assuming the data format is supported in the desired language.
  Ultimately, you need a lot of libraries to sustain this--the cross product
  of `|languages| x |formats|`.  Not to mention the glue and conversion loss.
  A time-wasting headache.
- *Efficiency*: If one program uses one format/language and some other program
  uses another, then there is usually manual coding effort and data copying
  in order to convert.  The problem gets worse the more pieces you glue
  together.  Modern software has a lot of pieces.
- *Flexibility*: By performing data transformations on a neutral representation,
  you can define common operations that apply across many data formats. For
  example, you can define *sort* or *transpose* operations that work
  independently of which format the data was originally in, or which data
  format it will eventually be stored as.

#### Components of Ellis

- *core*: The core in-memory representation.
- *codec*: Encoders and decoders for mapping various data formats into Ellis so
  that they can be manipulated by *core*.
- *stream*: Support for data streams so that you can easily encode or decode
  data from some source (e.g. file stream, or socket stream).

# Build prerequisites

- *meson* (recent). Unless your distro has a recent meson (probably 2016 or
  later), you should install via pip: *pip3 install meson*, or get it from
  source at http://mesonbuild.com/.
- *ninja* (>= 1.6). If your distro's version is older, the following directions
  will download and install a more recent version:
    - ```wget https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-linux.zip```
    - ```unzip ninja-linux.zip```
    - ```sudo install -o root -g root -m755 ninja /usr/bin/ninja
      # or put it somewhere else in your PATH```
- *boost-asio*
- *boost-system*
- *nghttp2* (>= 1.18.0). If you need to compile, use these flags for
  configure:
  *--with-jemalloc --enable-asio-lib*
  Note that *--with-jemalloc* is technically optional but will improve
  performance.
- *openssl* (>= 1.0.2). (needed by nghttp2)

Note that, if any of your build prerequisites do not come from standard distro
packaging, you will need also need to tweak the following env vars:

- *PKG_CONFIG_PATH* needs to be set only when you run *meson* and doesn't matter
  after that. It should be set to the directory containing the *.pc* files used
  by the prerequisite you built.
- *LD_LIBRARY_PATH* needs to be set whenever you actually load the Ellis
  library, such as when you run the unit tests with *ninja test*.  It should be
  set to the directory containing the built prerequisite libraries.

# Build instructions

#### First time builds

```
mkdir build
cd build
meson ..
ninja
```

#### Rebuilding

To rebuild at any time, you just need to rerun the last `ninja` command:

```
ninja
```

You can run this command from any directory you like.  For instance, to
rebuild from the top level directory, use the ninja `-C` option to point ninja
at the build directory:

```
ninja -C build
```

Also, there is nothing special about the directory name `build`.  If you
prefer a different name than `build`, this is not a problem, and you
can have different build directories with different configurations; meson and
ninja don't care.

#### Compiling with clang instead of gcc

It's the usual meson methodology:

```
mkdir build.clang
cd !$
CC=clang CXX=clang++ meson ..
ninja
```

#### Running tests

```
ninja test-valgrind
```

#### Running all static analysis and style checking

(Be aware that the below includes calls to cppcheck, clang-check, and clang-tidy,
which requires that you have such things installed on your system).

```
ninja check
```

If you don't have clang installed, for instance, you can use the individual
`cppcheck` target.
