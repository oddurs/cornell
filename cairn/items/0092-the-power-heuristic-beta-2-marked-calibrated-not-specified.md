---
id: 92
title: 'The power heuristic: beta = 2, marked CALIBRATED not specified'
type: optics
status: backlog
milestone: v0.8
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: s
---

The power heuristic raises every density to a power before normalising, which
sharpens the weighting toward whichever strategy was more likely. Veach found
that beta = 2 works well, said so, and gave no derivation, because there is
none — it is a number that came out of experiments.

So it is `windsor`'s CALIBRATED marker, exactly: a figure that was chosen
rather than derived, labelled as such, adjacent to the balance heuristic which
was derived. The difference between a specification and a calibration is the
difference between a model and a fit, and this project says which is which
every time.

- [ ] The comment says the balance heuristic is derived and this is not
- [ ] Both are available and the measured variance difference is quoted
