---
id: 136
title: Progressive photon mapping, and admitting that it is biased
type: optics
status: backlog
milestone: v1.4
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: transport
effort: xl
---

For specular-diffuse-specular paths, an unbiased estimator is not practically
available, and the field's answer is density estimation: gather photons within
a radius, which is biased, and shrink the radius as the render proceeds, which
makes it *consistent* — converging to the right answer in the limit while
being wrong at every finite step.

That distinction is exactly the kind this project should explain carefully
rather than gloss, because it is the first time the renderer knowingly returns
a wrong number and the reasoning for accepting it is good.

- [ ] Bias and consistency defined in the header, with the radius schedule
      shown to satisfy the conditions
- [ ] `converge` is expected to show a different slope, and says why
