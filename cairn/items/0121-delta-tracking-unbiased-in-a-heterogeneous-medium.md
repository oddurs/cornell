---
id: 121
title: Delta tracking, unbiased in a heterogeneous medium
type: optics
status: backlog
milestone: v1.1
labels:
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: media
effort: l
---

Sampling a collision distance in a medium of varying density has no closed
form. Delta tracking fills the medium with fictitious particles until it is
uniform, samples the uniform medium, and rejects — a trick from neutron
transport in the 1960s, which is where most of this machinery comes from and
which the file should say.

Derive the unbiasedness. It is the interesting part and it is what makes the
fictitious particles feel like a proof rather than a hack.

- [ ] Ratio tracking for transmittance, which is lower variance and also
      unbiased, with the difference between them explained
- [ ] `converge` shows the slope unchanged inside a medium
