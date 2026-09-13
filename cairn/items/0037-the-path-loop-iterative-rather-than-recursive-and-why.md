---
id: 37
title: The path loop, iterative rather than recursive, and why
type: optics
status: backlog
milestone: v0.2
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: m
---

The rendering equation is recursive and the implementation must not be. Carry
a throughput spectrum, loop, and accumulate — which is the same computation
with the stack made explicit, and which is what makes Russian roulette,
multiple importance sampling and (much later) a wavefront formulation
expressible at all.

The comment should show the recursive form and the iterative form next to each
other, once, and then never mention it again.

- [ ] No recursion anywhere in the integrator
- [ ] Maximum depth is a diagnostic limit, not a physical one, and says so
