---
id: 59
title: Tile-based threading with std::jthread, and nothing else
type: optics
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
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

- [x] Bit-identical output at 1 thread and at N — six renders at 1, 2, 3, 7,
      10 and 16 threads produce byte-identical PFM files
- [x] No mutable state shared between tiles other than the film, and not
      really the film either: tiles do not overlap, so no two threads touch
      the same accumulator and no lock is needed. The one shared thing is the
      atomic counter handing out tiles, and which tile a thread gets is a
      race on purpose
- [x] Two renders at different thread counts are bit-identical, which is
      item 0035's last criterion — now asserted by `./cornell verify` rather
      than measured once, at 2, 3, 7 and 16 threads against 1

## 2026-09-15

Six renders at 1, 2, 3, 7, 10 and 16 threads produce byte-identical PFM files, and ./cornell verify now asserts it on every run rather than it being measured once. Scaling on this machine: 1.84, 3.56, 6.78, 10.17 and 11.38 million paths per second at 1, 2, 4, 8 and 10 threads - 6.2x on ten, which tapers after four because the cores are not all the same. Tiles rather than scanlines: a square's rays are more alike so the same parts of the BVH stay in cache, and a row is as wide as the image so its slow region becomes one thread's problem. No lock on the film, not because the writes are atomic but because tiles do not overlap and the writes never collide. This closes item 0035's last criterion from v0.2, which the sampler was designed to make true.
