---
id: 46
title: Tone mapping is not physics, and is marked as such
type: optics
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
labels:
- admission
created: 2026-09-13
updated: 2026-09-14
priority: p2
area: colour
effort: s
---

Radiance is unbounded and a display is not. Everything that happens between
those two facts is a choice about appearance, and none of it belongs in the
renderer.

So: the film writes linear float, always. Tone mapping is a separate pass in a
separate file with a comment saying it is the one part of this project that
has no correct answer. Any figure quoted in the README comes from before this
step.

- [x] Every number in the README is measured pre-tone-map — the README's
      figures come from `./cornell` and `./cornell render`'s own output,
      which is printed from the linear data before this step runs, and
      cornell.pfm is written before it too
- [x] The default is documented as a choice, with its author named — the
      default is `clip`, which has no author and the file says so; the
      alternative is Reinhard, Stark, Shirley and Ferwerda, SIGGRAPH 2002,
      named because house rule 8 says a curve-fit must be

## 2026-09-14

The default is clip, and clipping has no author - it reproduces everything at or below the exposure reference exactly and turns everything above it white, which is what a photograph exposed for the walls does to the lamp, which is the comparison v1.0 has to make. The alternative is Reinhard, Stark, Shirley and Ferwerda 2002, L/(1+L), which never clips and never reaches white; it is not the default because it moves every value including the representable ones, and a render whose mid-greys have been moved cannot be compared against a measurement. Applied per channel rather than to luminance, which is a choice and a criticised one - it desaturates bright colours - and is taken so that the two operators differ in exactly one respect and the comparison between them means something.
