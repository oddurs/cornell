---
id: 59
title: Tile-based threading with std::jthread, and nothing else
type: optics
status: backlog
milestone: v0.4
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: transport
effort: m
---

The standard library has threads, so the project has threads, and house rule 4
survives.

Tiles rather than scanlines, because a tile has better ray coherence and
because the last scanline of a slow region otherwise stalls the whole frame.
The per-pixel-per-sample seeding from v0.2 means this changes the speed and
not the image, and a test asserts exactly that.

- [ ] Bit-identical output at 1 thread and at N
- [ ] No mutable state shared between tiles other than the film
- [ ] Two renders at different thread counts are bit-identical, which is
      item 0035's last criterion: the sampler was built to make this true
      and there were no threads yet to prove it with
