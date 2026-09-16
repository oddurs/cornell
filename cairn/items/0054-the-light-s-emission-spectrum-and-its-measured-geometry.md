---
id: 54
title: The light's emission spectrum, and its measured geometry
type: optics
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
priority: p0
area: scene
effort: s
---

The emitter is a real lamp with a real spectrum and a real size. All three
matter: the spectrum sets the colour of everything, the size sets the softness
of every shadow in the image, and the radiant flux sets the exposure that the
v1.0 comparison depends on.

- [ ] Flux is quoted in watts, derived from the measured radiance and area —
      **it cannot be, and the criterion assumed something that was not
      published.** Cornell states plainly that the source spectra "are
      relative only; their scales are arbitrary", so there is no measured
      radiance to derive a flux from. The derivation is written down —
      `Phi = pi * L * A`, which for the measured area is `0.042882 * L` watts
      — and the radiance that goes into it is CALIBRATED, chosen in the scene
      rather than in `cornell.hpp`, so that the file stays a record of what
      was published
- [x] The comment states what kind of lamp it was — a tungsten flood light
      with a UV filter and a diffusing glass plate. The data page does not
      say; Cornell's companion measurement library does, and the shape of the
      four published points independently confirms it

## 2026-09-15

The lamp type is not on the data page. Cornell's companion measurement library names it: boxsource.mat is 'the spectrum of the light source in the Cornell box (tungsten flood light with UV filter and diffusing glass plate)'. The four published emission points corroborate that independently - fitting a blackbody to the 500/600/700 nm values gives 3350 K with an rms residual of 0.73 on values of 8 to 18, and a tungsten flood runs at 3200-3400 K. The single disagreement is at 400 nm, where the fit predicts 3.10 and the published value is 0.0, and that is the UV filter named in the same sentence. Three sources agreeing and their one conflict explained by a documented component. The flux criterion could not be met: the same page says the spectra are relative only with arbitrary scales, so no absolute radiance was published and no flux in watts can be derived from it. The derivation is recorded and the radiance is marked CALIBRATED. Also found: the lamp is exactly coplanar with the ceiling, and the published ceiling carries a matching hole at the lamp's four corners, so the lamp fills a hole rather than hovering - which is a better answer than the 10 mm dodge box.hpp used in v0.2.
