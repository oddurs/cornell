---
id: 156
title: What the second code review found
type: chore
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-15
updated: 2026-09-15
priority: p1
area: verification
effort: m
---

A review of v0.4 against this project's own house rules. The two serious
findings were bugs in `bvh.hpp` and went in their own commit; this item is
the rest, which is mostly house rule 6 — figures and prose that stopped being
true when the scene changed underneath them.

## What was wrong

**`--lamp` had become a dead flag.** Rewiring `render()` to `cornell::box()`
left `RenderSettings::tungsten` written by the argument parser and read by
nothing, while `print_witnesses` went on advertising it. `--lamp a` and
`--lamp d65` produced identical images. It works again, and item 0045's
demonstration is reproducible again with it.

**Two "Measured" tables in `render.hpp` were stale copies.** Both were taken
on the grey test rig `box.hpp` used to hold, with a different camera, and
neither was re-run when the box arrived. The scale table quoted a mean
radiance the program had stopped printing at all. House rule 6's exact
failure mode, four commits old, found by review rather than by the rule.

**`scene.hpp` said "any acceleration at all" is not modelled**, about a
hundred and fifty lines above an `intersect` that traverses a BVH, and its
opening still said the BVH *will* replace the loop.

**An orientation hint was right by luck.** `Vec3{0.6, 0, -0.8}` for the short
block's (290,114)→(240,272) face has a dot of 0.331 with that face's actual
normal, where every other hint in the file agrees to 0.996. The z components
have opposite signs; only the x term saves it. It is `{0.95, 0, 0.3}`, which
is what it was meant to be.

**`spec.hpp` named the wrong point** — "from the centre of the floor", about
the point directly beneath the *lamp*.

**The CI guard guarded nothing.** `if ./cornell verify >/dev/null || [ $? -ne
127 ]` — an unknown subcommand exits 1 from the usage path and never 127, so
the condition was always true and verify ran twice.

## What the re-measurement found

The convergence slope on the real box is **−0.509** against a theoretical
−0.5, from RMSE of 0.55733, 0.29074, 0.14059 and 0.06770 at 16, 64, 256 and
1024 spp.

And the scale-invariance claim, moved out of prose and into `./cornell
verify`, immediately failed — informatively. At 0.001x, 105 of 4096 pixels
differed. At 1024x, 1/1024x, 65536x and 1/65536x, none did.

Multiplying a coordinate by 1024 changes its exponent and leaves every
mantissa bit alone, so the scaled room is exactly the original room. 0.001 x
552.8 is not one thousandth of 552.8, it is the nearest double to it, so that
room is a slightly *different* room and a pixel on a silhouette may land on
the other side of an edge. The transport has no length in it; powers of ten
do.

## Acceptance criteria

- [x] Every figure in a header matches what the program prints now
- [x] No flag is advertised that does nothing
- [x] The scale and convergence claims are checks rather than paragraphs
- [x] `make strict` passes and `./cornell verify` holds
