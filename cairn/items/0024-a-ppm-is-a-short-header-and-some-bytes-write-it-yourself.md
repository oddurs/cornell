---
id: 24
title: A PPM is a short header and some bytes; write it yourself
type: instrument
status: planned
milestone: v0.1
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: instrument
effort: s
---

## What it witnesses

Nothing. It is the plumbing that makes everything else visible, and it exists
here so that house rule 4 is satisfied from the first commit rather than after
the first temptation.

## What it prints

A binary PPM. Later, a PFM for the linear float data, because quoting a number
from an 8-bit image is quoting a number that has been through a tone curve.

## Acceptance criteria

- [ ] No image library, now or ever
- [ ] The linear float output is the one the instruments read; the 8-bit one
      is for looking at
