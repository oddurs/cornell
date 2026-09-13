---
id: 63
title: 'apps/chi2: catch a sampling routine lying about its own density'
type: instrument
status: backlog
milestone: v0.5
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
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

- [ ] Adaptive binning so that grazing-angle bins are not starved
- [ ] Runs over every BSDF at several roughnesses and incident angles
- [ ] Part of `./cornell verify`; a failure is a build-breaking event
