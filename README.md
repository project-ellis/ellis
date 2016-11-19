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
