---
id: 153
title: transport.hpp is three files in a trenchcoat
type: chore
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-14
updated: 2026-09-15
priority: p2
area: prose
effort: m
---

The item's premise was that `transport.hpp` at 621 lines was an outlier
against 253 for the next largest file. That figure had rotted, which is house
rule 6 happening to the roadmap rather than to a header. The largest file in
the project is now `cornell.hpp` at 680 lines, and nobody filed an item about
it, because nobody re-measured.

That is worth keeping straight, because it changes the conclusion. Length was
never the test. `cornell.hpp` is 680 lines of one idea — a box somebody built
and measured, transcribed with every figure derived from the transcription —
and splitting it would produce two files that have to agree about the same
table. Rule 8 says one idea per file, and it means it literally.

## What was actually separable

Russian roulette, which is what the item said. It is now `roulette.hpp`: the
expectation algebra, the argument for choosing `q` from the throughput, the
argument for not starting at the first bounce, and the measured trade —

      rho    estimate    sample sd    seconds / 10⁵ paths
      on      0.5   1.997040     0.586          0.05
      off     0.5   2.000000     2.1e-08        3.27
      on      0.9   9.952060     8.97           0.15
      off     0.9  10.000000     0              3.28

— together with `roulette_start_depth` and `survival_probability`, which was
`largest_component` capped at 1 at the call site and is now one function whose
name says what the number is for.

The item raised the counter-argument itself: a termination rule read apart
from the loop it terminates may be worse than a long file. The test of that is
what the loop looks like afterwards, and it is three lines that say what they
do. `transport.hpp` keeps the depth limit, because that one is a *diagnostic*
and belongs with the loop it guards, and it keeps the cavity table, because
that measures the transport rather than the roulette.

621 to 540, and one more file that is one idea.

## What did not need to happen

Rule 8 is not amended. The alternative criterion — "the file says why 600
lines is the right size for it and rule 8 is amended to match" — was written
on the assumption that the rule was about size. It is not, and `cornell.hpp`
is the demonstration.

## Acceptance criteria

- [x] Either it is split, or the file says why 600 lines is the right size
      for it and rule 8 is amended to match
