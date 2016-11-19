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
If you want to build from a directory besides *build*, you can run *ninja -C
build*. Note additionally that if you prefer a directory besides build, you can
use that too; meson doesn't care what directory you build from, as long as it
gets its own directory.
