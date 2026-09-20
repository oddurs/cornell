---
id: 162
title: 'Verify: the rough conductor finally vanishes'
type: verify
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
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

## 2026-09-19

Measured. The rough conductor vanishes: directional albedo exactly 1.0000000000 at every roughness and angle, compared with == on the sheet, because with reflectance 1 nothing is absorbed at any facet and the estimate has no variance.

The item's second sentence, proved rather than asserted: orders two and up come to 0.693516 at roughness 1 head on, and the single-scattering model was short of 0.693160. They differ by 5.6e-04, so the light 0086 accounted for as masked or reflected into the surface is exactly the light the walk brings back.

The important finding is that the furnace is the WEAK half of this check and I had not seen that until the table existed. At reflectance 1 the weight never changes, so any walk that terminates by leaving upward returns exactly 1 - including one that scatters in entirely wrong directions. The energy test cannot see a direction error at all. What can is splitting by scattering order: the first-order share has to equal the single-scattering albedo, which torrance_sparrow.hpp computes from D, G2 and G1 sharing none of the walk's arithmetic. They agree to 5.6e-04. That is precisely the check that would have caught 0161's Lambda sign bug, which conserved energy perfectly and was out by 0.216 at roughness 1 and sixty degrees.

On the chi2 gap I flagged before starting: it is narrower than it looked and the reasoning is now in verify.hpp. chi2 loses the walk by design (0160), but the walk's first scattering event is pinned to the single-scattering model by the order table, and that model IS covered by chi2 at every roughness from 0.001 up in both frames. So what no sampling test reaches is orders two and above, and what covers those is energy - which cannot be lost or invented at any roughness. Not full cover, but named cover rather than a hole.

Calibration is the column beside the answer: the single-scattering model is the same surface with the light dropped instead of followed, so it is what the walk would look like if it stopped at one bounce, and it is 0.307 where the walk is 1. Nothing had to be built to be caught.

CI runs both the walk furnace and the order table.
