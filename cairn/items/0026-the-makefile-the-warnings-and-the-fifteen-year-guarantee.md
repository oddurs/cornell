---
id: 26
title: The Makefile, the warnings, and the fifteen-year guarantee
type: chore
status: planned
milestone: v0.1
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

- [ ] Builds clean under both clang and gcc
- [ ] No CMake, no configure, no submodules
