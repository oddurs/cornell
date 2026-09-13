---
id: 139
title: The aperture is a polygon, so the bokeh is a polygon
type: optics
status: backlog
milestone: v1.5
labels:
- thesis
- derivation
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: camera
effort: s
---

An out-of-focus highlight takes the shape of the aperture, because it *is* the
aperture, projected. Six blades give hexagonal bokeh. Stopping down a
nine-blade lens gives the nonagon everyone recognises and nobody can name.

There is no bokeh parameter. There is a blade count.

- [ ] `grep -ri bokeh include/` finds only an explanation that it is absent
- [ ] Blade count and curvature are the only inputs
