---
id: 51
title: Find out exactly what Cornell published, and what units it is in
type: spike
status: done
milestone: v0.4
assignee: Oddur Sigurdsson
labels:
- thesis
created: 2026-09-13
updated: 2026-09-15
priority: p0
area: scene
effort: m
---

## Question

What measured data actually exists for the Cornell box, in what units, and
what was measured versus what has been passed down by repetition?

## Why it has to be answered first

The entire premise of naming the project `cornell` is that the box is a real
object with published measurements. If the geometry in circulation is a
reconstruction and the reflectances are somebody's guess, the project needs to
know that before it builds a validation milestone on top of it.

Note against temptation: the figure "2.7 metres on a side" was floated during
the naming of this repository and is an invention. It is recorded here so that
nobody finds it in the history and believes it.

## Options

The Cornell Program of Computer Graphics published geometry, spectral
reflectances for the walls, and the emission spectrum of the light, along with
photographs and radiometric measurements. Establish which of those are
primary, and read the 1984 paper — Goral, Torrance, Greenberg and Battaile,
*Modeling the Interaction of Light Between Diffuse Surfaces* — rather than a
summary of it.

## What would settle it

A written note in this item recording, for each figure the project will use:
the source, the units, and whether it was measured or chosen. That note
becomes the opening comment of `cornell.hpp`.

## Answer

The data exists, it is primary, and it is measured. It is also no longer on
the web, the units are not stated anywhere on the page, the box is not
rectangular, the light's spectrum is four numbers, and the photograph the
project intends to be compared against is not the one from 1984.

### Where it is

`http://www.graphics.cornell.edu/online/box/data.html`, published by the
Cornell Program of Computer Graphics, last updated 2 February 2005.

**That URL is dead.** It now redirects to a Cornell Bowers marketing page
with no data on it. The page survives in the Internet Archive:

    https://web.archive.org/web/2018/http://www.graphics.cornell.edu/online/box/data.html

This is not a footnote. A project whose premise is "the box is a real object
with published measurements" has just discovered that the publication has
gone, and it settles a design question that was going to come up anyway: the
figures get **embedded in the repository**, cited, with the archive URL
beside them. House rule 4 says this should still build in fifteen years, and
the last fifteen have already taken the source off the internet.

### The units, which are not stated

Nowhere on the page does it say what the geometry is in. The numbers are bare:
the floor runs from 0 to 552.8.

They are millimetres, and it can be established from the camera block, which
*does* carry units by convention:

    focal length   0.035        35 mm — a normal lens
    width, height  0.025 0.025  25 mm square film
    position       278 273 -800

A camera 800 of something away with a 0.035 m lens is 800 mm away. So the box
is roughly **0.556 × 0.5488 × 0.5592 metres** — about eighteen inches, a thing
you could carry.

This disposes of the note against temptation at the top of this item. "2.7
metres on a side" is an invention and is wrong by a factor of five.

### The box is not a box

    floor, far edge     552.8
    floor, near edge    549.6
    ceiling             556.0

Three different widths for what a diagram would draw as one. The page says
why, plainly: *"The geometry has been measured from the physical Cornell Box.
The surfaces are therefore not perfectly perpendicular."*

This is the single best thing in the data set, and it is the reason the
project is named after this box rather than drawing an idealised one. The
asymmetry is not noise to be cleaned up — it is what makes the geometry a
measurement instead of a specification, and `cornell.hpp` must carry all
three numbers rather than averaging them into one.

### The reflectances

Measured, at 4 nm intervals from 400 to 700 nm. 76 samples, three surfaces:

    white   0.343 to 0.777
    green   0.092 to 0.481
    red     0.040 to 0.657

The red wall's *maximum* reflectance is 0.657, at the long-wavelength end,
and it never drops below 0.040. It is not `#ff0000` and nothing like it: it is
a broad, gentle curve that happens to be higher in the red. That is item
0053's whole point and it is visible in the numbers before anything is
rendered.

The page states: *"Surfaces are assumed to be Lambertian."* Assumed. Not
measured to be. That is item 0061's admission, and the data set is already
making it.

### The light, which is the weakest part

    constant reflectance   0.78
    emission spectrum      400 nm  0.0
                           500 nm  8.0
                           600 nm  15.6
                           700 nm  18.4

Four numbers, no units, and nothing below 400 nm or above 700 nm. Everything
else in this data set is sampled at 4 nm and the thing that determines the
colour of the entire image is sampled at 100 nm.

It is what exists, so it is what gets used, and `cornell.hpp` will say so
loudly. The interpolation between those four points is a *choice this project
makes*, not data, and it has to be marked as one — which makes the light the
largest single source of uncertainty in the v1.0 comparison.

### Which photograph

The page describes photographs of the box *"in its current configuration"*,
taken with a liquid-cooled Photometrics PXL1300L CCD at 12 bits through seven
narrow-band filters, with dark current subtracted and flat-field correction
applied.

That is not the 1984 apparatus. Goral, Torrance, Greenberg and Battaile
photographed a box in 1984; this data describes a box, in the same room,
re-measured for a modern re-release, and the two are not required to be the
same object. The blocks in this geometry, for instance, are a diffuse
configuration that the 1984 paper's figures do not show.

So v1.0's claim has to be phrased against *this* data set and *these*
photographs, not against the 1984 plate. The project's argument is unharmed —
it is still a comparison against somebody else's measurement of a real object,
made before any of this was written — but saying "the 1984 photograph" would
be wrong, and the README currently implies it.

### The camera, and a check that it is self-consistent

    position    278 273 -800
    direction   0 0 1
    up          0 1 0

Looking down +z with +y up makes +x the camera's **left**, and the wall the
data calls "Left wall" is the one at large x, and it is the red one. The
naming is from the camera's point of view and it agrees with the geometry,
which is a small thing and a reassuring one: somebody checked.

### What is measured, what is chosen

For `cornell.hpp`, every figure falls into one of three buckets:

| figure | source | status |
| --- | --- | --- |
| wall geometry | measured from the physical box | MEASURED |
| wall reflectances, 400–700 nm at 4 nm | spectrometer | MEASURED |
| light geometry | measured | MEASURED |
| light emission, 4 points | measured | MEASURED |
| light emission *between* those points | this project | CALIBRATED |
| light emission outside 400–700 nm | this project | CALIBRATED |
| absolute radiometric scale of the light | not published | CALIBRATED |
| "surfaces are Lambertian" | Cornell's assumption | ASSUMED |
| units of the geometry | inferred from the camera block | INFERRED |
