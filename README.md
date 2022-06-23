# ymerge

A Source-Based Package Manager for Arch Linux.

```
Usage: ymerge [options] [pkg...]

A Source-Based Package Manager for Arch Linux.

Options:
    --help -h             print help

    --nocolor      turn off colors for produced shell output
    --quiet -q     reduce output
    --remove -R    remove (uninstall) packages
    --srcinfo      only print srcinfo but do not build (unless --makepkg is also specified)
    --sync -s      update local package database
    --verbose      increase verbosity of output

Examples:
    Update the package database:
    $ ymerge --sync

    Install the "xmake" and "build2" packages:
    $ ymerge xmake build2

    Remove the "xmake" and "build2" packages:
    $ ymerge --remove xmake build2
```

## Bootstrapping from an Existing Arch Linux System

Download the
[PKGBUILD](https://github.com/flying-dude/curated-aur/blob/main/pkg/ymerge/PKGBUILD)
and then use
[makepkg](https://wiki.archlinux.org/title/Makepkg)
to install the package:

```
wget https://raw.githubusercontent.com/flying-dude/curated-aur/main/pkg/ymerge/PKGBUILD
sudo pacman --sync base-devel
makepkg --syncdeps --install
ymerge --help
```

## Compile the Source Code

Clone the github repository and then use CMake to build the sources:

```
git clone --recurse-submodules https://github.com/flying-dude/ymerge
cd ymerge
pacman --sync base-devel clang cmake fmt nlohmann-json
cmake -S . -B build
cmake --build build
./build/ymerge --help
```
