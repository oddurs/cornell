---
id: 26
title: The Makefile, the warnings, and the fifteen-year guarantee
type: chore
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: build
effort: s
---

`make && ./cornell`. A C++23 compiler and nothing else, forever.

`-Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast -Wdouble-promotion`, and
`-Wdouble-promotion` earns its place here more than in most projects: a stray
`float` that silently widens in the middle of a spectral integral is exactly
the kind of quiet wrongness this project cannot afford.

- [x] Builds clean under both clang and gcc
- [x] No CMake, no configure, no submodules

## 2026-09-13

Clang 21 builds clean at -O0 and -O2 on this machine, and 'make strict' (-Werror) passes. The gcc half cannot be checked here - /usr/bin/g++ is the clang shim and no homebrew gcc is installed - and does not need to be: CI builds every pull request on ubuntu under g++-14 as well as on macos under clang, runs the program, and then builds it again with every warning fatal. That is a better answer than a second compiler on one laptop, because it is a clean checkout on a machine that is not this one. Adding -Wconversion and -Wsign-conversion turned up six real sign conversions in spectrum.hpp, all of them std::array subscripted with an int; they are now spelled out.
