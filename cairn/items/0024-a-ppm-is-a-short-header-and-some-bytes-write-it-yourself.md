---
id: 24
title: A PPM is a short header and some bytes; write it yourself
type: instrument
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
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

- [x] No image library, now or ever
- [x] The linear float output is the one the instruments read; the 8-bit one
      is for looking at

## 2026-09-13

Round-tripped: PFM writes the bottom row first (3,4,5 before 0,1,2 for a 3x2) and PPM the top row first, which is the flip the two formats disagree about. PPM clips 2.0 to 255 and -1.0 to 0, and linear 0.5 encodes to 186 under gamma 2.2 - sRGB would give 188. The worst disagreement between the two curves is 9 codes at linear 0.0013.
