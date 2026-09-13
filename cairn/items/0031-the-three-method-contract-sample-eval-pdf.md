---
id: 31
title: 'The three-method contract: sample, eval, pdf'
type: optics
status: backlog
milestone: v0.2
labels:
- foundation
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: bsdf
effort: m
---

## What it is

The interface every surface in the project implements, and the most
consequential design decision after the spectral one.

    sample(wo, u) -> { wi, f, pdf }     draw a direction, and say how likely
    eval(wo, wi)  -> f                  what does this surface do
    pdf(wo, wi)   -> density            how likely would sample() have been

The third method looks redundant and is not. It is what makes multiple
importance sampling possible in v0.8, and a project that omits it discovers
this two months later, at the cost of every material it has written.

## What it must derive

Nothing, but it must make the χ² test in v0.5 expressible: `sample` and `pdf`
are two independent claims about the same distribution, and the whole point of
separating them is that a machine can check they agree.

## Acceptance criteria

- [ ] `std::variant` and `std::visit`, not virtual dispatch — a jump table is
      what you would write by hand for a tagged union
- [ ] No BSDF may be added without all three methods
