---
id: 70
title: 'Fireflies: diagnose rather than clamp'
type: spike
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-16
priority: p2
area: transport
effort: m
---

## Question

What does this project do about fireflies — single pixels left enormously
bright by a rare high-energy path?

## Why it has to be answered first

The universal answer is to clamp indirect radiance, which works, looks fine,
and is *biased*: it silently removes energy, it breaks the convergence slope
that v0.5 measures, and it is exactly the kind of quiet dishonesty this
project exists to avoid.

## Options

Clamp and admit it, breaking the unbiasedness claim. Sample better, so the
paths that cause them are found more often — which is what multiple importance
sampling in v0.8 is for, and which fixes the cause. Or report them: a
diagnostic that says which path produced the outlier.

## What would settle it

If the fireflies that remain after v0.8 come from a single identifiable
mechanism, fix the mechanism. If clamping is still wanted afterwards it goes
in the tone mapping pass, where non-physical things are already admitted to
live, and never in the integrator.

## Answer

**Nothing, on purpose, and here is the mechanism.**

Not clamping is not a preference. `./cornell converge` measures a slope, a
clamp removes energy, and removed energy is a systematic error that no number
of samples fixes — the slope would bend and the instrument built two items ago
would be measuring a renderer that had stopped converging to the right answer.
So the answer is a diagnosis and a diagnostic, and both are measurements.

## What a firefly is here, measured

The right statistic is a **share**: what fraction of everything a pixel
received came from its single largest sample. An even estimator gives every
pixel 1/N. One carried by a single lucky path gives it 1. It is scale-free and
it does not mistake the lamp — bright in every sample — for an outlier.

At 128 x 128, over every lit pixel:

      spp   roulette    median share   99th pct   over a half
       16   on                1.0000     1.0000    3051 of 3178
       64   on                1.0000     1.0000    7734 of 8684
      256   on                0.4584     1.0000    6227 of 13901
     1024   on                0.1545     1.0000    1325 of 15251
     4096   on                0.0497     0.6236     291 of 15536
     4096   off               0.0362     0.4953     151 of 15545

At sixteen samples the **median** pixel gets all of its value from one sample.
At four thousand it still gets five percent from one, against the 0.00024 an
even estimator would give — two hundred times its fair share — and one pixel
in a hundred gets more than sixty percent from one.

That is not a tail of rare accidents. It is the shape of the estimator: a path
contributes only if it happens to strike the lamp, and the lamp subtends about
0.045 steradian of the 6.28 a bounce can go into. About seven paths in a
thousand find it per bounce. Everything else returns zero.

## Which makes the mechanism v0.8's, not the roulette's

The roulette is an amplifier and not the cause, and the difference is
measurable both ways.

It *is* an amplifier. `./cornell replay 117,93,29 200` shows one in a single
table: three bounces decay the throughput to 0.40, the roulette runs at
q = 0.295, the path survives and is scaled back to **1.0**, and then it finds
the lamp twice at full weight. Turn the roulette off and the worst
sample-to-pixel ratios in a 200 x 200 render fall from 57.8 to 2.8.

It is *not* the cause. With the roulette off the median share at 4096 samples
is 0.0362 against 0.0497 — better, and nowhere near 0.00024. The sparsity
survives the roulette being switched off entirely, because it was never about
the roulette.

So the single identifiable mechanism the item asked for is the absence of
light sampling, and v0.8 is where that is fixed. This item does not wait for
v0.8 to find out; it says in advance what the fix has to do, which is what
makes v0.8 falsifiable: **the median share at 4096 samples must fall towards
1/4096, and if it does not, multiple importance sampling did not do what it
was added for.**

## And the diagnostic

`./cornell render --outliers N` prints the census above and then the N
brightest single samples as addresses, in the spelling `./cornell replay`
takes. `replay` runs one of them again, alone, on one thread, and prints every
bounce: the distance, the throughput, the roulette's q, and what it hit.

That needed a hook inside `radiance()`. `replay.hpp` had said, one item
earlier, that it was "not written until it is needed" and that when it was it
would be "a template parameter with a no-op default and it costs nothing". It
is, and it does: 1.41 s before and 1.41 s after, over three runs of a 200 x 200
render at 64 samples on one thread.

## If clamping is ever wanted

It goes in `tonemap.hpp`, where non-physical things are already admitted to
live and where nothing quotes a figure from the result. Never in the
integrator.
