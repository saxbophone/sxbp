# sxbp ![sxbp](sxbp.png "sxbp")

Experimental generation of 2D spiralling lines based on input binary data.

This is the command-line-interface for [libsaxbospiral](https://github.com/saxbophone/libsaxbospiral).

## Dependencies

You will need:

- A compiler that can compile ISO C99 code
- [Cmake](https://cmake.org/) - v3.0 or newer
- [libsaxbospiral](https://github.com/saxbophone/libsaxbospiral) - the library that does all the spiral generation work
- [Argtable 2](http://argtable.sourceforge.net/) - must use v2, v1 and v3 will not work

> ### Note:

> These commands are for unix-like systems, without an IDE or other build system besides CMake. If building for a different system, or within an IDE or other environment, consult your IDE/System documentation on how to build CMake projects.

## Basic Build

```sh
cmake .
make
```

## Recommended Build with optimisation

Add two custom options to CMake to build the program in release mode (with full optimisation)

```sh
cmake -DCMAKE_BUILD_TYPE=Release .
make
```

## Install

This command might require `sudo`, but check your system configuration. For example, it installs to `/usr/local/` by default, which is user-writable on OSX if you use Homebrew, so not requiring admin privileges.

```
make install
```
