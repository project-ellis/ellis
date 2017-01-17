# Building

# Build prerequisites
- *meson* (recent). Unless your distro has a recent meson (probably 2016 or
  later), you should install via pip: *pip3 install meson*, or get it from
  source at http://mesonbuild.com/.
- *ninja* (>= 1.6). If your distro's version is older than, the following directions
  will download and install a more recent version:
    - *wget https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-linux.zip*
    - *unzip ninja-linux.zip*
    - *sudo install -o root -g root -m755 ninja /usr/bin/ninja*
- *boost-asio*
- *boost-system*
- *nghttp2_asio* (>= 1.18.0). If you need to compile, use these flags for
  configure:
  *--with-jemalloc --enable-asio-lib*
  Note that *--with-jemalloc* is technically optional but will improve
  performance.
- Libssl.

Note that, if any of your build prerequisites do not come from standard distro
packaging, you will need also need to tweak the following env vars:

- *PKG_CONFIG_PATH* needs to be set only when you run *meson* and doesn't matter
  after that. It should be set to the directory containing the *.pc* files used
  by the prerequisite you built.
- *LD_LIBRARY_PATH* needs to be set whenever you actually load the Ellis
  library, such as when you run the unit tests with *ninja test*.  It should be
  set to the directory containing the built prerequisite libraries.

## Build instructions

```
mkdir build
cd build
meson ..
ninja
```

You can use `ninja` to rerun the build at any time, from any directory you like.

For instance, to rebuild from the top level directory, use the ninja `-C`
option to lead ninja to the build directory:

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
