---
id: 27
title: 'README: the argument, before there is anything to show'
type: prose
status: done
milestone: v0.1
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-13
priority: p2
area: prose
effort: m
---

## What it has to explain

Why a person would read this. The two theses, stated plainly enough that
somebody who has never rendered anything understands the claim and can tell
whether it has been met.

## What it must not do

Promise. At v0.1 the program draws a sphere on a black background, and the
README should say that while explaining what it is going to be.

## Acceptance criteria

- [x] The gold claim is stated in the first screen
- [x] Every figure quoted is marked with the command that printed it

## 2026-09-13

Both quoted output blocks are checked against the program rather than transcribed: a script re-runs ./cornell and ./cornell render and compares. The 'forty bytes of header' in the milestone description is wrong - the PPM header is 15 bytes and the PFM header is 16 - so the README says fifteen.

## 2026-09-13

Went stale exactly the way house rule 6 predicts, and a code review found it rather than the rule. The status section still said v0.1 and 'draws a sphere on a black background' three commits after v0.2 was finished; the quoted render block claimed '480 x 320, 64 samples per pixel, 9.8 million rays', a line the program had stopped printing and a resolution height_for can no longer produce; and the item count said 133 against 135. Both quoted blocks are now checked against the program by a script rather than transcribed, and the two machine-dependent lines - elapsed time and rate - are marked as such, because a figure that cannot be reproduced should not be presented as one that can.
