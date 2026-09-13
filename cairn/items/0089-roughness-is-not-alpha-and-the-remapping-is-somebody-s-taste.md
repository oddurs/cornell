---
id: 89
title: Roughness is not alpha, and the remapping is somebody's taste
type: optics
status: backlog
milestone: v0.7
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: bsdf
effort: s
---

The parameter in the distribution is alpha. The parameter artists use is
"roughness", and the two are related by a squaring that Disney introduced in
2012 because it makes a slider feel linear.

That is a perceptual convenience with a named author and no physical content,
which makes it exactly the kind of thing house rule 8 says must not be
disguised as a law. It lives at the scene boundary, next to the other
conversions, and it says whose taste it is.

- [ ] `alpha` is what the physics sees; `roughness` never appears inside a BSDF
- [ ] The remapping is cited
