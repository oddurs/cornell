---
id: 162
title: 'Verify: the rough conductor finally vanishes'
type: verify
status: backlog
milestone: v0.7
created: 2026-09-19
updated: 2026-09-19
priority: p0
area: verification
effort: s
---

## The claim

A rough metal with reflectance 1, in a uniform environment, returns radiance 1
in every direction and disappears — at every roughness, not only at the smooth
end.

This is item 0085 asked again after 0161, and it is the sentence this whole
milestone was arranged to be able to say. 0085 measured the failure; this
measures the repair.

## How it is checked

`./cornell furnace --bsdf conductor`, the same table, with the deficit column
at zero. And `./cornell furnace --table`, where the `masked` and `below`
channels should no longer be losses: the light in them has been followed and
comes back in `escaped`.

## What failure looks like

Too dark still means energy is being dropped somewhere in the walk. **Too
bright means the walk is double-counting**, which is the new failure mode this
repair introduces and which the old model could not have.

The deficit at roughness 1, head on, is 0.693. That is the number to beat, and
the README quotes it.
