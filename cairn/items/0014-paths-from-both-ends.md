---
id: 14
key: v1.4
title: Paths from both ends
type: milestone
status: backlog
depends_on:
- 13
created: 2026-09-13
updated: 2026-09-13
priority: p2
---

Some light paths are essentially impossible to find by walking backwards from the camera. A caustic under a glass of water is the standard example: the camera sees a bright patch on a table whose brightness depends on a path that must pass through a specular surface in exactly the right place.

Bidirectional path tracing walks from both ends and joins in the middle. Progressive photon mapping gives up unbiasedness for the paths that need it and says so.

This is where the promise that a caustic is not a feature gets its hardest test.
