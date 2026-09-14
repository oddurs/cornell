---
id: 152
title: What the first code review found
type: chore
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p1
area: verification
effort: m
---

The first review of the whole of v0.1 and v0.2 against this project's own
house rules. It found eleven things. Three were faults in the machinery that
checks the work and went in their own commit; one was the cavity table in
`transport.hpp` describing code that had stopped existing, which went in
another. This item is the rest.

The most useful result is not any single finding. It is that the two worst
ones — a figure nine orders of magnitude wrong, and a CI step exercising a
thousandth of what it claimed — were both *self-checking machinery that had
quietly stopped checking*, and neither was going to be caught by anything
except somebody reading it.

## What was wrong

**The photopic peak is 555 nm, not 550.** `si.hpp` says 555. `render.hpp`
said 550 in a comment, directly above a line that prints 555, because
`bin_of` lands in the bin centred on it. The two visibly disagreed.

**`height_for` divided a constant by itself.** `width * film_side /
film_side` is the identity function dressed as an aspect-ratio derivation,
and would have gone on returning the width if the film ever stopped being
square — which is the exact bug v0.1 shipped an ellipse over.

**"The samples are jittered" claimed a stratification that is not
performed.** Each sample takes two uniforms from its own stream, so they are
i.i.d., and four can land in the same corner. Unbiased and N^-½ either way,
but not the better thing the word claims — and it means `warp.hpp`'s
argument for the concentric mapping is preserving a stratification the
renderer never establishes.

**`film.hpp`'s clamp cited a case that cannot occur.** It said a `u` of zero
puts a wavelength on the upper edge; it puts it at three quarters of the
span.

**`integer_or` had signed overflow in it**, which is undefined behaviour, so
`./cornell render 99999999999` was accepted as a positive-looking width.

**Six declarations nothing called.** House rule: `si.hpp` is the *only* file
where dead code is sanctioned.

## What was decided rather than deleted

`Wavelengths::pdf()` stays and now says why: house rule 3 is that a sample
and its density are one object, and `cie.hpp` divides by this one in v0.3.
`separated()`, `Film::count()` and the BSDF's `pdf()` already carried their
justifications.

`basis::cos_theta` was the interesting one. It took world-space arguments,
both callers work in the local frame, so both wrote `std::fabs(w.z)` by hand
— leaving a helper sitting unused beside a comment claiming it existed to
stop people doing that. It is `abs_cos_theta` in `bsdf.hpp` now, where the
convention it depends on is defined, and both callers use it.

## Acceptance criteria

- [x] Every figure in a header matches what the program prints
- [x] No dead code outside `si.hpp`, or a written reason for the exception
- [x] `make strict` still passes
