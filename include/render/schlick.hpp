// schlick.hpp — the fit, in the file next door to the law.
//
// Christophe Schlick, "An Inexpensive BRDF Model for Physically-based
// Rendering", Computer Graphics Forum 13(3), Eurographics 1994. One line:
//
//      R(theta) = R0 + (1 - R0) (1 - cos theta)^5
//
// where `R0` is the reflectance at normal incidence and `theta` is measured
// from the normal. It is in every real-time renderer in the world, it is in
// most offline ones, and it is a **curve fit**.
//
// It sits beside `fresnel.hpp` on purpose. House rule 8 says a file is named
// for a phenomenon or a person and that the naming carries information, and
// this is its clearest case: `fresnel.hpp` is a law, exact, derived from
// boundary conditions Maxwell would later justify. `schlick.hpp` is a
// five-parameter-free approximation to the file next to it, published a
// hundred and seventy-one years later, and it is here so that a reader can
// see the trade rather than inherit it.
//
// ── What it gets right, which is most of it ──────────────────────────────
//
// The fifth power is not arbitrary and it is not fitted. It falls out of
// insisting on three things at once: the value at normal incidence, the value
// at grazing, and that the curve be flat near the normal — exact Fresnel has
// zero first derivative at `theta = 0`, which is why a window looks uniform
// until you are well off axis and then goes bright quickly. The lowest power
// that can satisfy all three with one term is 5.
//
// For a dielectric it is very good on average and worse than its reputation
// at the extreme. Measured against exact Fresnel, unpolarised, over 90,001
// angles by `./cornell verify`:
//
//      medium            n        R0       max error   mean error  worst at
//      water           1.330    0.02006     0.05992     0.01086     83.8 deg
//      window glass    1.500    0.04000     0.03569     0.00922     85.0 deg
//      diamond         2.417    0.17197     0.07544     0.01338     84.5 deg
//
// About one percent on average, from one line. That is why it won. The "under
// one percent" the approximation is usually credited with is the *mean*: the
// maximum is three to eight percent, and every one of those maxima is at 84
// or 85 degrees, because the curve is pinned at both ends by construction and
// the last few degrees before grazing are where it has the most room to be
// wrong.
//
// ── What it gets wrong, which is the point of the file ───────────────────
//
// A metal's refractive index is complex, and `R0 + (1 - R0)(1 - cos)^5` has
// nowhere to put an imaginary part. Schlick's derivation assumes a real
// index; the usual practice is to compute `R0` from the complex index and
// then use the dielectric shape anyway, which is an approximation nobody
// writes down.
//
// Measured, at the illustrative complex indices below rather than at a metal,
// because the metals arrive with their own tables and their own citation:
//
//      index              R0       max error   mean error  worst at
//      n=0.2  k=3.0     0.92337     0.01544     0.00461     76.5 deg
//      n=1.1  k=7.0     0.91762     0.10112     0.02121     83.4 deg
//      n=0.05 k=4.2     0.98933     0.00604     0.00157     79.6 deg
//
// Which is the result worth having, and it is not the one this comment
// originally guessed. The error on a conductor is **not a multiple of the
// error on glass**. It is 0.006 at one index and 0.101 at another — better
// than glass in the first case and three times worse in the second — because
// a conductor's reflectance curve is not the shape Schlick fitted, and how
// badly that shows depends entirely on where the index sits. There is no
// correction factor, and a renderer that uses this for metal is accepting an
// error it cannot bound without computing the thing it was avoiding.
//
// ── Why it is in this project at all ─────────────────────────────────────
//
// Nothing uses it. It is not a default, it is not an option, and no material
// reaches for it — `./cornell verify` is the only caller, and what it does
// with it is measure the error above so that the figures in this comment
// cannot rot.
//
// It is here because the alternative to showing the trade is inheriting it.
// Every renderer a reader has used computes Fresnel this way, most of them
// without saying so, and a project whose whole argument is "derive, never
// declare" owes it to a reader to put the declaration next to the derivation
// and print the difference.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Polarisation, more thoroughly than next door. Exact Fresnel gives two
// reflectances and this gives one, so it cannot even be asked the question
// Brewster's angle answers: Schlick's curve has no zero, for any `R0`. The
// angle at which a wet road stops glaring is not representable here.

#pragma once

#include <cmath>

#include <render/fresnel.hpp>

namespace render {

namespace schlick {

// The fit. `R0` is the caller's business — for a dielectric it is
// `((n-1)/(n+1))^2` and for a conductor it is the complex form, and the
// second of those is already an admission that this curve is being used
// outside what it was derived for.
constexpr double reflectance(double r0, double cos_theta) {
    const double one_minus = 1.0 - cos_theta;
    const double squared = one_minus * one_minus;

    // The fifth power as `x^2 * x^2 * x`, which is three multiplies rather
    // than a `pow`. The whole argument for this file is that it is cheap, so
    // it would be strange to spend a transcendental on it.
    return r0 + (1.0 - r0) * squared * squared * one_minus;
}

// The normal-incidence reflectance the fit is pinned to, taken from the exact
// equations rather than from a second formula. `fresnel.hpp` has the closed
// form and it works for a complex index; using it here is what makes the
// comparison a comparison of *shapes*.
inline double r0_of(const Index& eta_t) { return normal_incidence(eta_t); }

} // namespace schlick

} // namespace render
