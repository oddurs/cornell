---
id: 84
title: Visible-normal sampling, with its exact density
type: optics
status: done
milestone: v0.7
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-18
priority: p0
area: sampling
effort: l
---

Sampling the normal distribution directly wastes most of its samples on facets
that face away from the viewer. Sampling the *visible* normals — the
distribution weighted by how much of each facet the viewer can see — is
strictly better, and Heitz's 2018 formulation does it by sampling a hemisphere
stretched into an ellipsoid, in about fifteen lines.

The pdf is exact, which matters more than the speed: an approximate pdf here
would pass casual inspection and fail the chi2 instrument, which is the whole
reason that instrument was built two milestones ago.

- [x] chi2 passes at roughness 0.001 through 1.0
- [x] The stretch-and-unstretch geometry is explained, not just performed

## 2026-09-18

Three files: visible_normals.hpp (Heitz 2018), torrance_sparrow.hpp (the BRDF that uses it), and GreyRough in the Bsdf variant so chi2 and 0085's furnace have something to point at.

D_v = G1 <wo.m> D / cos(theta_o) is normalised because of 0083: the covering requirement that fixed Lambda is, read backwards, the statement that D_v integrates to one. Deriving G instead of picking it is what pays for the sampler being exact.

A real bug, caught by moments rather than by eye: the squash must be applied along the axis lying in the plane of wo and the normal, and basis.hpp returns whatever Duff's branch-free construction points at. With an arbitrary frame the sampled normals were plausible and the first three moments were out by 40 percent at grazing, while agreeing exactly at normal incidence where the squash is the identity. Heitz builds the frame explicitly and this is why.

On criterion 1, precisely how it was met. The chi2 histogram is 32x64 uniform in (mu, phi), so a bin is 0.06 wide in mu; at alpha 0.001 the lobe is 2e-6 of mu. Measured before any of this was written: total density 0.00094 where it should be 1, statistic 1.1e9. That is a resolution wall, not a model error, and no subdivision clears it because the lobe narrows faster than any fixed grid. So the low-alpha rows are histogrammed in the flattened frame, where the distribution has the same width at every roughness. The stretch factor is 0.8/alpha and NOT 1/alpha deliberately: stretching by exactly 1/alpha would undo what the sampler did, the roughness would cancel out of the sample path, and an unstretch by alpha^2 would pass. The residue of 0.8 leaves nothing cancelling. The direct un-stretched rows run from alpha 0.1 up and cover what the stretch cannot see.

chi2's sub_cells went 6 -> 12. Six was tuned for Lambert, which is smooth; a visible-normal density stops dead at wo.m = 0, a boundary that does not follow the grid, so a clipped cell could report every one of 36 sub-samples dead while the sampler drew into the live sliver - a false IMPOSSIBLE DRAW. Twenty changes the p-values in the fourth decimal, so twelve is converged.

f cos_i / pdf = F G2/G1 exactly - no D, no cosines, no 4. Checked over 166025 draws at three roughnesses, worst disagreement 5.55e-16, max weight exactly 1.000000. The divides are written out anyway, which is house rule 3's whole point.

README corrected: it said the microfacet model 'will fail all three' instruments. It fails the furnace and passes chi2 and converge, which is the discrimination worth having.
