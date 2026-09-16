---
id: 69
title: 'Verify: no NaN survives to the film, asserted'
type: verify
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-16
priority: p2
area: verification
effort: s
---

One NaN in an accumulation buffer poisons that pixel for the rest of the
render, and NaN propagates silently through every operation that touches it.

Assert on non-finite values in debug builds, at the point where a sample is
added to the film, with the pixel and sample index in the message — which,
because of the reproducible seeding from v0.2, is enough to replay exactly
that path.

- [x] The assertion message contains enough to reproduce the path
- [x] A `--replay pixel,sample` flag exists and re-runs one path

It is `./cornell replay x,y,sample`, a subcommand rather than a flag, because
it does not modify a render — it is the only thing the program does when it is
asked.

## What the address is worth

`sampler.hpp` made a path's entire random state a function of `(pixel,
sample)`, addressed rather than dispensed, and the reason given at the time was
that the thread count must not change the image. This item is the second thing
that bought. A path has an address, so a path can be run again — alone, on one
thread, with a breakpoint in it.

So the assertion prints the address in the form the replay takes, and the
replay runs the path three times and compares bit for bit, because a
reproduction that is not reproducible is not one.

What it does not print is the bounces. That needs a hook inside `radiance()`,
and the transport is not getting one for a diagnostic while something cheaper
works: one deterministic path under one thread is a breakpoint away from every
bounce it took. If that stops being enough the hook is a template parameter
with a no-op default and costs nothing — but it is not written until it is
needed.

## What it found

**The NaN this project knows how to make does not reach the film.**
`camera.hpp` says a degenerate `up` hint produces a NaN direction "left to
propagate rather than be papered over". It propagates into the ray and stops
there: every comparison against a NaN is false, so the intersection finds
nothing, the path escapes, and the film receives a perfectly finite zero.

The failure mode is a black image, not a poisoned one, and **no assertion at
the film can catch it**. That is now a row on the inspection sheet and a
paragraph in `camera.hpp`, rather than an assumption about how far a NaN
travels.

**The predicate has to be `isfinite` and not `x != x`.** A NaN fails the
self-comparison and an infinity passes it, and an infinity poisons a running
sum exactly as thoroughly. Both are checked.

**And the assertion is on in the ordinary build.** The Makefile does not
define `NDEBUG` and should not: four comparisons per *sample* sit beside a
ray-scene intersection, and a renderer that trades a NaN check for that is
trading the wrong way. The sheet also scans a finished film — 460,800 values
over every bin and every tristimulus — because an assertion only speaks when
it is violated, and a check that relies on silence cannot tell "no NaN
occurred" from "the assertion was compiled out".
