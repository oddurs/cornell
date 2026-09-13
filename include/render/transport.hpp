// transport.hpp — the assumption the whole program is standing on, written
// down before any of the program is.
//
// This file will hold the rendering equation, and it does not hold it yet:
// that is v0.2, item 0029. What it holds now is the admission, and the
// admission comes first on purpose. A reader who wants to render a soap
// bubble should find out that they cannot in the first file they open, not in
// the third week, and a simplification nobody stated is not a simplification.
// It is a lie with a schedule.
//
//      THIS IS GEOMETRIC OPTICS: MAXWELL'S EQUATIONS IN THE LIMIT WHERE EVERY
//      FEATURE IS LARGE COMPARED WITH A WAVELENGTH.
//
// In that limit light travels in straight lines, energy adds, a surface is
// described by how much it sends where, and the whole of image synthesis
// becomes one integral equation that can be estimated by following paths. It
// is an extraordinarily good approximation — it is why a photograph of a
// pinhole camera looks like a pinhole camera — and it is exact nowhere.
//
// Visible light is 360 to 830 nanometres. So the limit holds when the things
// light meets are much larger than about half a micron, which covers a
// plywood wall, a sphere, a lens, a droplet of water and a grain of pigment,
// and does not cover a soap film, an oil slick, the ridges on a beetle's
// shell, the pits on a CD, the coating on a camera lens, or the edge of any
// aperture at all. Those are the cases below, and each one says what it would
// cost to have.
//
// ── What this model cannot produce, and what each would cost ──────────────
//
// The four are kept as items under the `later` milestone — 0143 to 0146 —
// and the sketches here are the short version of those.
//
//   1. POLARISATION.
//
//      Light has an orientation as well as an intensity, and a surface
//      responds to the two orientations differently. `fresnel.hpp` in v0.6
//      will compute both and then average them, which is the exact moment the
//      information is discarded.
//
//      What it costs: a wavelength stops carrying one number and carries four
//      — the Stokes parameters — so the path state quadruples. Every BSDF
//      stops being a scalar and becomes a 4 × 4 Mueller matrix, which means
//      every material in the project is rewritten, and the matrices have to
//      be rotated into a common reference frame at every bounce, which is a
//      new source of subtle sign errors.
//
//      What it buys: the glare off water at a steep angle, which a polarising
//      filter removes and this renderer cannot; the polarisation pattern of
//      the sky, which bees navigate by; stress birefringence in glass.
//
//   2. FLUORESCENCE.
//
//      A wavelength arrives, is absorbed, and leaves as a longer one. This
//      model has no mechanism for that at all: `Reflectance` multiplies a
//      `Radiance` wavelength by wavelength, which is a diagonal matrix, and
//      fluorescence is precisely the off-diagonal terms.
//
//      What it costs: the diagonal becomes a full re-radiation matrix — a
//      Donaldson matrix — so a surface is described by a square table rather
//      than a curve, and the four wavelengths a path carries can no longer be
//      chosen once at the camera, because the surface decides where the
//      energy goes next. Hero wavelength sampling, which `spectrum.hpp` is
//      built on, does not survive that unchanged.
//
//      What it buys: the optical brighteners in white paper and detergent,
//      which is why a white shirt is *brighter* than white; highlighter pens;
//      anything under a blacklight; most safety clothing.
//
//   3. INTERFERENCE.
//
//      Two beams at the same wavelength add their powers here. Real ones add
//      their amplitudes, which can cancel.
//
//      What it costs: light has to carry a phase as well as a magnitude, and
//      a coherence length, since sunlight only interferes with itself over a
//      few microns. A thin film needs the optical path difference through it
//      and a sum over multiple internal reflections. It is a different
//      quantity being transported, not an extra term.
//
//      What it buys: the colours in a soap film and an oil slick, the
//      structural colour of a beetle or a butterfly or a peacock — none of
//      which is a pigment — and the anti-reflective coating on every lens
//      this project will model in v1.5, which works by interference and will
//      have to be faked.
//
//   4. DIFFRACTION.
//
//      Light bends around edges, and an aperture spreads the beam that passes
//      it by an angle of roughly λ/d. The pinhole in `camera.hpp` has zero
//      size, so in this model it spreads light infinitely well and produces a
//      perfectly sharp image — which is the wrong answer by an amount that
//      goes to infinity as the hole closes.
//
//      What it costs: an aperture stops being a hole that passes rays and
//      becomes a wave propagation, or — cheaply and approximately — a
//      scattering function obtained from the Fourier transform of the
//      aperture. The cheap version is tractable and is what the few renderers
//      that do this at all use.
//
//      What it buys: the starburst around a street light at night, which is
//      the shape of the diaphragm blades; the rainbow off a CD; the softness
//      of every real pinhole photograph; and the resolution limit of every
//      lens ever ground, which is the reason telescopes are large.
//
// ── One more, which is not a wave effect ──────────────────────────────────
//
// Light here arrives instantly. The rendering equation as it will be written
// in v0.2 has no time in it, so every path is in equilibrium and nothing can
// be in flight. Time-of-flight imaging, and the photograph of a light pulse
// crossing a bottle, need a transport equation with a delay term and a film
// with a temporal axis. It is not on the roadmap at all, which is itself
// worth saying, since the absence of an item is the easiest omission to
// mistake for an oversight.
//
// ── What this file will contain ───────────────────────────────────────────
//
// The rendering equation, once, written out where it can be read, and the
// estimator that solves it. That is v0.2. Nothing is declared below yet, and
// the emptiness is deliberate: the argument is the part that had to exist
// first, because everything written after it inherits the limit.

#pragma once

namespace render {

// Deliberately empty. See above, and item 0029.

} // namespace render
