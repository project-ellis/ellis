# Building

## Build requirements
- Meson (http://mesonbuild.com/download.html)
  - TLDR: unless your distro has a really recent Meson, do:
    - *pip3 install meson*
- Ninja (https://ninja-build.org/)
  - TLDR: If your distro's version of ninja is older than 1.6, then do:
    - *wget https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-linux.zip*
    - *unzip ninja-linux.zip*
    - *sudo install -o root -g root -m755 ninja /usr/bin/ninja*

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

### Compiling with clang instead of gcc

It's the usual meson methodology:

```
mkdir build.clang
cd !$
CC=clang CXX=clang++ meson ..
ninja
```

### Running tests

```
ninja test-valgrind
```

### Running all static analysis and style checking

(Be aware that the below includes calls to cppcheck, clang-check, and clang-tidy,
which requires that you have such things installed on your system).

```
ninja check
```

If you don't have clang installed, for instance, you can use the individual
`cppcheck` target.
