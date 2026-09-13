---
id: 103
title: 'Verify: the hollow sphere'
type: verify
status: backlog
milestone: v0.9
created: 2026-09-13
updated: 2026-09-13
priority: p0
area: verification
effort: s
---

## The claim

The dielectric handles the inside-outside boundary correctly in both
directions.

## How it is checked

A glass sphere with a slightly smaller sphere of negative radius inside it —
a bubble, a shell of glass with air in the middle. Rays cross four boundaries
and the sign is different at each.

## What failure looks like

The classic wrong result is a sphere that looks like a solid glass ball
instead of a shell, or a shell with a dark ring. It is the oldest test in the
hobby and it catches the oldest bug, and it is here because passing it is the
difference between glass that is right and glass that looks right.
