---
id: 45
title: Chromatic adaptation, and why a white point is a choice
type: optics
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-15
priority: p2
area: colour
effort: s
---

A sheet of paper looks white under noon daylight and under a tungsten bulb,
and its spectrum is enormously different in the two cases. The eye adapts;
the film does not.

Bradford adaptation, with the honest statement that this is a model of
perception bolted onto a radiometric renderer, that it is not physics, and
that it is where "white balance" lives.

- [x] The adaptation step is separable and can be switched off — its own
      file, its own matrix, applied in one place, `--no-adapt`
- [x] With it off, a tungsten-lit render is orange, and that is correct —
      measured, mean linear sRGB R/B of 7.97 against 1.009 adapted. D65
      adapted and unadapted are bit-identical, because adapting an
      illuminant to itself is the identity

## 2026-09-15

Illuminant A is derived rather than tabulated, from the CIE's own defining formula - which has an obsolete c2 (1.435e7 nm K) and an obsolete temperature (2848 K) fossilised in it, designed to cancel. Verified three ways: the formula gives x 0.44757 y 0.40744 against a published 0.44758, 0.40745; modern Planck at the modern label of 2856 K gives 0.44753, 0.40743, which is the cancellation working; and modern Planck at the written 2848 K gives 0.44815, 0.40760, visibly a different light, so the relabelling is not cosmetic. It needed a constexpr exp, since std::exp is not constexpr before C++26 - range-reduced, sixteen Taylor terms, measured at 5.0 ulps over the range A uses. Bradford checks: D65 to D65 is the identity to 1e-15, and A's white adapts onto D65's chromaticity to 1e-12. Rendered: tungsten unadapted has a mean linear sRGB R/B of 7.97, adapted 1.009, and D65 is unchanged by the switch.
