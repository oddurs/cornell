---
id: 135
title: Bidirectional path tracing
type: optics
status: backlog
milestone: v1.4
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: transport
effort: xl
---

Walk a path from the camera and a path from the light, and connect every
vertex of one to every vertex of the other. Veach again, and the natural
extension of everything MIS was built for.

The hard part is not the walking; it is that each connection is a different
sampling strategy for the same path, so the MIS weights are over a family of
strategies rather than two. Getting those weights right is the entire
difficulty and the reason the implementation is worth reading.

- [ ] The weight computation is derived, and tested independently of the
      renderer
- [ ] Verified against unidirectional on scenes where both work
