---
id: 37
title: The path loop, iterative rather than recursive, and why
type: optics
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
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

- [x] No recursion anywhere in the integrator
- [x] Maximum depth is a diagnostic limit, not a physical one, and says so

## 2026-09-13

Verified against an exact answer rather than by eye. A closed cavity whose walls all emit Le and reflect rho has isotropic radiance L = Le + rho*L, so L = Le/(1-rho). Over 200000 paths: rho 0.25 -> 2.61e-12 relative error, 0.5 -> 1.11e-16, 0.75 -> 1.11e-16, 0.9 -> 3.05e-13. Those are not small errors from many samples, they are no error: cosine-sampling a Lambertian makes f*cos/pdf exactly rho per draw, so every path returns the same geometric series and the estimator has zero variance. The test therefore proves the transport is right and proves nothing about noise. The residual at rho = 0.9 is the depth limit's truncation, 0.9^256 = 1.9e-12, which is the bias the limit's own comment predicted.

## 2026-09-13

The previous commit's message quoted a cavity table that was never added to transport.hpp, and closed this item with both criteria unticked. The edit script died on a Python syntax error before running, and the shell line that followed it was separated by a newline rather than by &&, so every later command ran as though the edit had succeeded. CLAUDE.md house rule 6 says to assert the anchor exists when editing prose programmatically; the asserts were there and were never reached. The lesson is the chaining, not the asserts: an edit step has to be able to stop the commit that depends on it.

## 2026-09-13

The cavity table was measured before Russian roulette existed and was never re-run after it landed. It claimed relative errors of 1e-12 to 1e-16 and 'no error at all' while the shipped code gives 1.6e-4 to 1.3e-3 - nine orders of magnitude - and sat four commits in the same file as a roulette table reporting a sample sd of 0.586, which is incompatible with zero variance. House rule 6 says reconcile every figure after any change to the model; this is exactly that failure, and a code review found it rather than a re-run. Re-measured with standard errors: every albedo from 0 to 0.9 lands within 1.02 se of exact, so the integrator is unbiased and the errors are Monte Carlo noise. The zero-variance property of f*cos/pdf survives - all the variance is the roulette's 1/q scaling - and the depth limit now contributes nothing, since at rho 0.9 a path reaches 256 bounces with probability about 1e-12.
