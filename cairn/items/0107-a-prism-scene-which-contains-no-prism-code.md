---
id: 107
title: A prism scene, which contains no prism code
type: optics
status: backlog
milestone: v0.9
labels:
- thesis
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: scene
effort: s
---

A triangular block of SF11, a narrow beam of D65, and a white card.

The scene file contains a shape, a glass name, and a light. It contains no
dispersion setting, no spectrum for the output, and nothing that mentions a
rainbow. The spread on the card is a consequence of the Sellmeier coefficients
of a real glass, and if it ever needs help, the model is wrong.

This is the project's showpiece and the second most direct demonstration of
the thesis after the metals.

- [ ] `grep -ri "prism\|rainbow\|dispers" include/` finds only Sellmeier's
      own file and its explanation
- [ ] The angular spread is measured and compared with the analytic
      prediction from the glass's datasheet
