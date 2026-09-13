---
id: 69
title: 'Verify: no NaN survives to the film, asserted'
type: verify
status: backlog
milestone: v0.5
created: 2026-09-13
updated: 2026-09-13
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

- [ ] The assertion message contains enough to reproduce the path
- [ ] A `--replay pixel,sample` flag exists and re-runs one path
