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
//      have to be approximated rather than computed.
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
// ── The equation ─────────────────────────────────────────────────────────
//
// Kajiya, SIGGRAPH 1986, "The Rendering Equation". As he wrote it:
//
//      I(x, x') = g(x, x') [ e(x, x') + ∫  p(x, x', x'') I(x', x'') dx'' ]
//                                       S
//
//      I   the intensity of light passing from x' to x
//      g   the geometry term: zero if x and x' cannot see each other, and
//          otherwise falling off with the square of the distance between them
//      e   the light emitted from x' towards x
//      p   the scattering term: how much light arriving at x' from x''
//          continues towards x
//      S   the union of all surfaces in the scene
//
// It is an integral over *surfaces*, which is the formulation that makes the
// visibility explicit — `g` is where the shadow lives — and it is the form
// that bidirectional methods in v1.4 will need, because a path built from
// both ends is naturally a sequence of points rather than of directions.
//
// The form this project computes in is the same equation with the variable
// changed from points to directions:
//
//      L(x, wo) = Le(x, wo) + ∫ f(x, wi, wo) L(x, wi) (n·wi) dwi
//                             H²
//
// The two are related by the Jacobian of that change of variables,
//
//      dw = cos(theta') dx'' / r²
//
// which is exactly Kajiya's `g`. It has not gone anywhere: it has been
// absorbed into the measure, and the visibility it carried is now performed
// by casting a ray. That is the whole reason the directional form is the one
// to implement — the integral runs over a hemisphere that is always the same
// shape, instead of over a set of surfaces that changes with the scene, and
// the occlusion test is a function call rather than a term.
//
// ── Which file owns which term ───────────────────────────────────────────
//
//      Le(x, wo)       `scene.hpp` — `Surface::emission`, and `emitted()`,
//                      which returns it only on the side the normal faces
//
//      f(x, wi, wo)    `bsdf.hpp` — the three-method contract every surface
//                      signs. `lambert.hpp` is the only model so far and
//                      `fresnel.hpp` in v0.6 is the one the project is for
//
//      (n·wi)          `basis.hpp` — `cos_theta`, and the tangent frame the
//                      BSDF is evaluated in
//
//      L(x, wi)        the recursion, and therefore this file. It is resolved
//                      by casting a ray — `scene.hpp` finds what is there,
//                      `waechter.hpp` makes sure the ray does not find the
//                      surface it started on
//
//      ∫ … dwi         `warp.hpp` chooses the directions and states their
//                      density; `sampler.hpp` supplies the variates that
//                      choosing consumes. The division by that density is
//                      written out at the point of use and never cancelled,
//                      which is house rule 3 and item 0032
//
// ── The list with no code ────────────────────────────────────────────────
//
// Soft shadows. Colour bleeding. Caustics. Ambient occlusion. Depth of field.
// Glossy reflection. Indirect illumination. Contact shadows.
//
// **None of them has any code.** Not a routine, not a flag, not a term. They
// are what solving the equation above honestly *looks like*, and a reader
// will not believe that without being told plainly, because every renderer
// they have used has a checkbox for at least four of them.
//
// A soft shadow is the light's solid angle being partly blocked, which falls
// out of an area emitter and a visibility test. Colour bleeding is `f` being
// spectral and the recursion having more than one bounce. A caustic is a path
// that happens to go light → specular → diffuse → eye. Ambient occlusion is
// an approximation *to* this equation that exists because the equation was
// too expensive in 1998; computing the equation and then adding ambient
// occlusion to it is adding an approximation of a thing to the thing.
//
// The acceptance criterion for this file is that a search of `include/` and
// `apps/` for the names of those features finds nothing, and it is checked on
// every pull request rather than asserted here — the pattern lives in
// `.github/workflows/ci.yml`, outside the tree it searches, which is the only
// way a grep for a word can be run over a file that has to be free of it.
//
// If it ever fires, the project has stopped making its second claim and
// should stop printing it.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// The equation as written assumes light leaves a surface from the point it
// arrived at (no subsurface transport — v1.3), at the same instant (nothing
// is in flight, and nothing fluoresces), and at the same wavelength (no
// fluorescence — see the list above). The wavelength assumption is why
// `Reflectance` multiplies a `Radiance` component by component: a diagonal
// matrix, where fluorescence would need a full one.
//
// It also assumes light travels unchanged between surfaces, which is the
// subject of item 0041 and the next thing this file has to admit.
//
// ── What this file will contain ──────────────────────────────────────────
//
// The estimator that solves the equation above: the path loop, iterative
// rather than recursive, carrying a throughput. That is item 0037, and it is
// the next commit. The equation had to be written down before the thing that
// solves it, because everything in the loop is a term from it.

#pragma once

namespace render {

// Deliberately empty. See above, and item 0029.

} // namespace render
