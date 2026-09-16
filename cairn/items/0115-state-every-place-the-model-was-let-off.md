---
id: 115
title: State every place the model was let off
type: prose
status: backlog
milestone: v1.0
labels:
- admission
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: prose
effort: m
---

The honest half of the validation, and the section that makes the other half
worth reading.

Collected from the admissions accumulated through every milestone: the walls
are Lambertian and real paint is not; the light is a uniform emitter and a
real lamp is not; polarisation is averaged; the room outside the box is
absent; the plywood has a thickness the model ignores; the camera has a lens
this render does not.

Each with an estimate, where one can be made, of how much it contributes to
the residual.

- [ ] Every item labelled `admission` in this backlog appears here or is
      explicitly retired
- [ ] `cairn list --view honesty` is empty when this closes
- [ ] The plywood being assumed Lambertian, which is item 0061's second
      criterion. Oren-Nayar departs most from Lambert at grazing angles and
      in corners, which is where the colour bleeding is and what the
      comparison is measuring. Either the error is below the measurement
      uncertainty — with a number — or this becomes an item
