# Project Ellis

JSON, MessagePack, CBOR, YAML, Avro, ORC, Parquet...  All fine choices, but
if your favorite is different than theirs, oh well... convert to theirs.

Once upon a time, it was like that with picture file formats.  And then we
got image libraries that import or export whatever, presenting the programmer
with a neutral in-memory representation.

Project Ellis seeks to achieve a similar gain for general data formats, and
includes some convenient tools for interoperability.  Specifically, Project
Ellis seeks to provide cross-language bindings for a common in-memory
representation of hierarchical data, along with encoder and decoder plugins
for common data formats, schemas, and streaming mechanisms.  In addition to
the library, command line tools like `ellisconv` and `ellispipe` are provided.

#### Advantages to this approach

- *Simplicity*: Many libraries require a particular format. If you have a
  different format, use a different library. Different programming language?
  Different library.  That's going to be a lot of libraries (cross product of
  `|languages| x |formats|`, not to mention glue and conversion loss. You have
  better things to do with your time.
- *Flexibility*: By performing data transformations on a neutral representation,
  you can define common operations that apply across many data formats. For
  example, you can define *sort* or *transpose* operations that work
  independently of which format the data was originally in, or which data format
  it will eventually be stored as, as long as the data has some fundamental
  structure.

#### Components of Ellis

- *core*: The core in-memory representation.
- *codec*: Encoders and decoders for mapping various data formats into Ellis so
  that they can be manipulated by *core*.
- *stream*: Support for data streams so that you can easily encode or decode
  data from some source (e.g. file stream, or socket stream).

## Build prerequisites

- *meson* (recent). Unless your distro has a recent meson (probably 2016 or
  later), you should install via pip: *pip3 install meson*, or get it from
  source at http://mesonbuild.com/.
- *ninja* (>= 1.6). If your distro's version is older, the following directions
  will download and install a more recent version:
    - ```wget https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-linux.zip```
    - ```unzip ninja-linux.zip```
    - ```sudo install -o root -g root -m755 ninja /usr/bin/ninja # or put it somewhere else in your PATH```

#### Custom requirements building

If any of your build prerequisites do not come from standard distro
packaging, you may need to tweak the following env vars:

- *PKG_CONFIG_PATH* needs to be set only when you run *meson* and doesn't matter
  after that. It should be set to the directory containing the *.pc* files used
  by the prerequisite you built.
- *LD_LIBRARY_PATH* needs to be set whenever you actually load the Ellis
  library, such as when you run the unit tests with *ninja test*. It should be
  set to the directory containing the built prerequisite libraries.

#### Debian unstable

To take care of build requirements, you just need to run:

```
sudo apt-get -y install meson ninja-build
```

#### Fedora 24 and 25 packages

To take care of build requirements, you just need to run:

```
sudo dnf -y install meson ninja-build
```

Note that on fedora you will substitute the **ninja-build** command instead of
the **ninja** command for the various build commands on this page.

## Build instructions

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

You can run this command from any directory you like. For instance, to
rebuild from the top level directory, use the ninja `-C` option to point ninja
at the build directory:

```
ninja -C build
```

Also, there is nothing special about the directory name `build`. If you
prefer a different name than `build`, this is not a problem, and you
can have different build directories with different configurations; meson and
ninja don't care.

#### Compiling with clang instead of gcc

If you want to use clang, it's the usual meson methodology:

```
mkdir build.clang
cd !$
CC=clang CXX=clang++ meson ..
ninja
```

#### Running tests
```
ninja test
```
or
```
ninja test-valgrind
```

#### Running static analysis and style checking

Be aware that the below includes calls to cppcheck, clang-check, and
clang-tidy, which requires that you have such things installed on your system.

```
ninja check
```

If you don't have clang installed, for instance, you can use the individual
targets such as `cppcheck`.

## Documentation

See the [doc](doc) folder for more documentation, such as:

* [Instructions for contributing](doc/CONTRIBUTING.md)
* [Coding standard](doc/coding_standard.md)
* [Design notes](doc/design.md)
