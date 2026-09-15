---
id: 47
title: What to do about negative RGB
type: spike
status: done
milestone: v0.3
assignee: Oddur Sigurdsson
created: 2026-09-13
updated: 2026-09-15
priority: p2
area: colour
effort: s
---

## Question

A saturated spectral colour converts to sRGB values outside [0,1], often
negative. Clipping changes the hue. What should happen?

## Why it has to be answered first

It determines whether the v0.9 prism image is honest or quietly clipped, and
the prism is one of the project's two showpieces.

## Options

Clip per channel; scale toward the white point preserving hue; compress the
gamut smoothly; or output a wider-gamut file and say so.

## What would settle it

Render the prism. Whichever option lets a reader see that the spread is
continuous and spectral, rather than three bands, is the right one.

## Answer

**Desaturate towards the white point at constant luminance.** Slide the
colour along the line joining it to white until nothing is negative, and
leave everything already inside the gamut exactly where it is.

### The experiment, without waiting for the prism

The spike said the deciding test was to render the prism, in v0.9. That was
not necessary: what the prism *shows* is a continuous sweep through the
spectral colours, and the sweep can be produced straight from the observer
with no refraction involved. 380 to 700 nm in 641 steps, each converted to
sRGB, each run through a candidate, each encoded to 8 bits:

    strategy                  distinct  longest flat  worst hue shift
    clip per channel               265           184         33.2 deg
    desaturate to the gamut        402            50          0.0 deg

First, the scale of the problem: **all 641** of those wavelengths have a
negative sRGB component. Not the saturated ones — every monochromatic colour
there is, because the spectral locus is a curve and the gamut is a triangle
inside it.

"Longest flat" counts consecutive wavelengths coming out as exactly the same
8-bit colour. Clipping produces a run of **184** — twenty-nine per cent of
the visible spectrum as one unchanging block. That is the "three bands rather
than a spectrum" this spike was written to prevent, and it turns up without a
prism being involved at all.

Clipping also moves the hue by up to 33 degrees. A clipped violet is not a
dimmer violet; it is a blue.

Desaturating shifts the hue by 0.0 degrees, and by construction rather than
by luck: a line towards `(y, y, y)` in linear RGB is a line towards the white
point in chromaticity, and hue is direction from the white point. What is
lost is saturation, which is the honest loss — a spectral green becomes the
most saturated green the display can produce, because that is the most
saturated green the display can produce.

### What was rejected, and why

*Clip per channel*: the measurements above.

*Compress the gamut smoothly*: the serious algorithms also move colours that
are already inside, so that the ones being brought in have somewhere to go.
This project's interior colours are measurements of a real box, and moving
them to make room for invented ones is the wrong trade for a renderer whose
last milestone is a comparison against a photograph.

*A wider-gamut file*: better in every respect except that it needs the reader
to have a display and a viewer that honour it, and this project writes a PPM.
It is partly taken anyway — `cornell.pfm` is written before any of this, with
its negatives intact, because a negative component is information.

### Where it lives

`apps/gamut.hpp`, next to `apps/tonemap.hpp`, because both are decisions
about appearance rather than statements about light, and neither may touch
anything a figure is quoted from.

## 2026-09-15

Settled without waiting for v0.9: the prism's content is a continuous spectral sweep, and that can be produced from the observer directly with no refraction. All 641 wavelengths sampled from 380 to 700 nm are outside sRGB - every monochromatic colour is. Clipping gives 265 distinct 8-bit colours with a flat run of 184 consecutive wavelengths, which is 29 per cent of the spectrum as one block and is exactly the banding this spike existed to prevent, plus a hue shift of up to 33.2 degrees. Desaturating towards white at constant luminance gives 402 distinct, longest flat 50, and a hue shift of 0.0 degrees by construction, since a line towards grey in linear RGB is a line towards the white point in chromaticity and hue is direction from the white point. Implemented in apps/gamut.hpp, applied after the PFM is written so the unmapped negatives survive in the file that exists to preserve evidence.
