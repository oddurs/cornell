---
id: 41
title: Admit that light travels unchanged between surfaces
type: prose
status: done
milestone: v0.2
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: prose
effort: s
---

The rendering equation as implemented assumes vacuum between every pair of
surfaces. No photograph was ever taken in one.

Say so in `transport.hpp`, name what it costs — fog, smoke, the shaft of light
through a window, the colour of deep water, the reason a glass of milk is
white — and point at v1.1 by name.

- [x] The admission is in the header, not only here

## 2026-09-13

CI caught the admission's own prose: the deep-water paragraph said a renderer without media 'has to fake that with a tint', and the grep that enforces transport.hpp's second claim does not distinguish prose from code. Reworded to 'approximate'. The same check is now in the local build gate as well - the pattern lives in the hook, which is outside the directories it searches - so the next one costs two seconds instead of a CI round trip.
