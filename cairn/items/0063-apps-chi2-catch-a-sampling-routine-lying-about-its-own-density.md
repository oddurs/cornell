---
id: 63
title: 'apps/chi2: catch a sampling routine lying about its own density'
type: instrument
status: done
milestone: v0.5
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-16
priority: p0
area: instrument
effort: l
---

## What it witnesses

That `sample()` and `pdf()` describe the same distribution.

Draw a few million directions from `sample()`, histogram them over the sphere,
integrate `pdf()` analytically over each bin, and compare with Pearson's
chi-squared test. Report a p-value.

This is the direct heir to `windsor`'s rule that analytic derivatives get
checked against finite differences, and it is the reason some renderers are
correct and most are not. Almost every BSDF bug in existence is a disagreement
between these two functions, and almost none of them are visible in an image
until they are compared against something.

## What it prints

    $ ./cornell chi2
    lambert                     p = 0.412   pass
    conductor  alpha=0.01       p = 0.297   pass
    conductor  alpha=0.40       p = 0.0000  FAIL   (see 0000)

## What it may not do

Use the BSDF's own sampling routine to compute the reference. The two claims
must be evaluated independently or the test is a tautology.

## Acceptance criteria

- [x] Adaptive binning so that grazing-angle bins are not starved
- [x] Runs over every BSDF at several roughnesses and incident angles
- [x] Part of `./cornell verify`; a failure is a build-breaking event

There is one roughness, because there is one BSDF. Every alternative of
`scene.hpp`'s variant is tested, at four incident angles and from both sides
of the surface.

## What it measured

**The pooling is real and visible.** 2048 bins over the sphere, half of them
zero-density because a reflector scatters into one hemisphere. Cells close
when they expect five draws, so the count depends on how many draws there are:

      draws     cells    dof
        256        49     48
       1024       185    184
       8192       821    820
      65536       992    991
    1048576      1024   1023

**Two exact facts a p-value could only hint at.** The three alternatives of
`Bsdf` share Lambert's sampling routine, so from one seed they must draw one
direction — over 2^20 draws, compared bit for bit, 0 differ. And a `wo` below
the horizon draws the mirror of the one above, because `lambert.hpp` negates
a z rather than sampling again: 0 of 2^20 differ from the negated draw. Both
would have shown up above as suspiciously equal chi-squareds, and
"suspiciously equal" is not a measurement.

**The calibration, which is the part worth keeping.** Two samplers that draw
exactly what Lambert draws and misreport the density. The gross one claims a
flat hemisphere and dies at p = 0. The subtle one claims cos^1.02 — two
percent too steep — and gives a chi2/dof of **1.2**, which looks fine, at
p = 5.7e-05.

A two-percent error in a density is invisible in an image, conserves energy
exactly, and passes the furnace. It does not pass this. And it is a question
of how hard you look: the same liar is passed at 2^19 draws with p = 0.04 and
caught at 2^20 with p = 6e-05. That is the test's power, and it is a property
of the sample count rather than of the threshold — which is why `verify` runs
the liars at four times the draws it runs the honest samplers at, and says so.

## What CI caught that clang could not

`0x9e37'79b9'7f4a'7c15ULL * std::uint64_t(n)` compiles silently under clang and
is a `-Wconversion` error under g++-14. On Linux `std::uint64_t` is `unsigned
long` and `ULL` is `unsigned long long`: the same width, a different type, so
the multiplication is a conversion between them. clang's headers make the two
agree and clang has nothing to say.

There is no gcc on this machine — `/usr/bin/g++` is clang wearing a different
name — so the two-compiler matrix is the only thing between this project and
a class of warning it cannot see locally. It did its job on the first pull
request that gave it something to find. The seeds are now `std::uint64_t`
constants with a note saying why.
