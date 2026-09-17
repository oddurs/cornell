// fresnel.hpp — how much light comes back off a boundary, and why gold is
// yellow.
//
// This is the file the project is for. Everything before it — the units, the
// spectra, the observer, the box, the instruments — exists so that this one
// can be believed, and the claim it makes is the whole argument:
//
//      You never type a colour. Gold is not `vec3(1.0, 0.766, 0.336)`. It is
//      a table of complex refractive index, measured with a spectrometer, and
//      the yellow falls out of the equations below or it does not fall out at
//      all. Swap the table for copper and the reflection turns pink because
//      of physics, not because somebody typed pink.
//
// ── Fresnel, 1823, and the best footnote in optics ───────────────────────
//
// Augustin-Jean Fresnel derived these from elastic-ether theory: a model in
// which light is a mechanical vibration of an invisible solid filling the
// universe, with a shear modulus and a density, and in which the two
// polarisations are vibrations along different axes of that solid.
//
// The model is entirely wrong. There is no ether. Michelson and Morley went
// looking for it in 1887 and found nothing, and by then Maxwell had already
// explained, in 1865, why Fresnel's answer was right anyway: what actually
// has to be continuous across the boundary is the tangential component of the
// electric and magnetic fields, and imposing that gives the same ratios
// Fresnel got from imagining a jelly.
//
// Forty years of being right for the wrong reason. It is worth a reader
// knowing that, because the equations are usually taught as though they were
// obvious, and the thing that makes them true is a boundary condition rather
// than a mechanism.
//
// ── The derivation, in the form this file implements ─────────────────────
//
// A plane wave arrives at a flat boundary between two media of refractive
// index `eta_i` and `eta_t`, at angle `theta_i` from the normal. Some of it
// reflects at `theta_i` and some refracts at `theta_t`, and Snell's law
// relates them:
//
//      eta_i sin(theta_i) = eta_t sin(theta_t)
//
// Matching the tangential fields across the boundary gives the ratio of
// reflected to incident *amplitude*, and it is different for the two
// polarisations because the two have their electric field pointing in
// different directions relative to the boundary:
//
//      s-polarised, field perpendicular to the plane of incidence:
//
//                eta_i cos(theta_i) - eta_t cos(theta_t)
//          r_s = ---------------------------------------
//                eta_i cos(theta_i) + eta_t cos(theta_t)
//
//      p-polarised, field parallel to it:
//
//                eta_t cos(theta_i) - eta_i cos(theta_t)
//          r_p = ---------------------------------------
//                eta_t cos(theta_i) + eta_i cos(theta_t)
//
// Those are amplitudes. What a detector measures is power, which is the
// squared magnitude:
//
//      R_s = |r_s|^2        R_p = |r_p|^2
//
// The "s" is for *senkrecht*, German for perpendicular, which is how the
// convention was named and why it is not "perpendicular" and "parallel"
// abbreviated to p and p.
//
// ── One function, two cases, and the difference is a data type ───────────
//
// The dielectric case and the conductor case are usually two functions in a
// renderer, with two derivations and two sets of branches. They are not two
// things. A conductor's refractive index is *complex* — `eta = n + ik`, where
// the real part refracts and the imaginary part absorbs — and every equation
// above is already written in terms of `eta` without caring what kind of
// number it is.
//
// So this file has one function, it takes `std::complex<double>`, and a
// dielectric is the case where the imaginary part is zero. That is not a tidy
// implementation trick; it is the actual physics, and writing it as two
// functions is what makes people think metals need a special model.
//
// ── Three things that fall out rather than being written ─────────────────
//
// **Total internal reflection.** Going from glass to air past the critical
// angle, `sin(theta_t)` exceeds 1, so `cos(theta_t)` is the square root of a
// negative number. In real arithmetic that is a branch somebody has to write.
// In complex arithmetic it is an imaginary cosine, the numerator and
// denominator of `r` become complex conjugates of each other, `|r|^2` is
// exactly 1, and nothing had to be tested for. There is no `if` in this file
// for it.
//
// **Brewster's angle.** `r_p` is zero when `eta_t cos(theta_i)` equals
// `eta_i cos(theta_t)`. Squaring both sides and substituting Snell:
//
//      eta_t^2 (1 - sin^2 theta_i) = eta_i^2 (1 - sin^2 theta_t)
//                                  = eta_i^2 - eta_t^2 sin^2 theta_i
//
// which rearranges to `sin^2 theta_i = eta_t^2 / (eta_i^2 + eta_t^2)`, and
// therefore
//
//      tan(theta_B) = eta_t / eta_i
//
// For air to glass at 1.5 that is 56.31 degrees, it is why polarising
// sunglasses kill the glare off a wet road, and `./cornell verify` finds it
// numerically from the equations rather than from that formula.
//
// It only exists for a real index. A conductor has no angle at which `r_p`
// vanishes — it has a *pseudo-Brewster* angle where the reflectance is merely
// least — and the difference is one of the things the complex case makes
// visible for free.
//
// **The sheen on everything at a grazing angle.** As `theta_i` approaches 90
// degrees, `cos(theta_i)` approaches zero, both ratios approach -1, and both
// reflectances approach 1. Every material in the universe becomes a mirror
// at grazing incidence: the sheen on a page held up to a window, the road
// ahead of you turning silver at sunset, the reason a wet surface is darker
// *and* shinier than a dry one — water fills the roughness, so the light that
// used to scatter diffusely now meets a flat boundary and obeys this.
//
// Nobody writes any of those. They are three consequences of two ratios.
//
// ── What is not modelled, and the irony of the file ──────────────────────
//
// **Polarisation**, and it is worth more than a sentence because the size of
// it is known exactly.
//
// These equations are stated per polarisation state. This renderer carries
// scalar radiance, so it averages the two — which is exactly right for
// unpolarised light meeting a surface for the first time, and wrong
// afterwards, because reflection *polarises* the light it reflects. The
// averaging happens at the call site rather than in here, so this file stays
// true and the approximation is visible where it is made.
//
// What it costs, computed rather than estimated. Unpolarised light, two
// bounces off glass, tracking the two states through both and then averaging
// once, against averaging after each:
//
//      first    second      tracked      averaged     ratio
//       0.00      0.00     0.00160000   0.00160000    1.0000
//      30.00     30.00     0.00198895   0.00172413    0.8669
//      45.00     45.00     0.00426907   0.00252405    0.5912
//      56.31     56.31     0.01094156   0.00547078    0.5000
//      70.00     70.00     0.04578120   0.02925555    0.6390
//      85.00     85.00     0.38981461   0.37552341    0.9633
//
// At two bounces at Brewster's angle the approximation returns **exactly
// half** the right answer, and that 0.5000 is the worst case over the whole
// grid of angle pairs. The reason is the thing Brewster's angle is: after one
// bounce at 56.31 degrees the light is completely s-polarised, and a second
// surface at the same angle reflects s strongly and p not at all — but the
// model has forgotten the light is polarised, so it applies the average a
// second time and loses the correlation.
//
// The same-plane case above is the *mild* one. Turn the second surface
// ninety degrees and the tracked answer at two Brewster angles is **zero** —
// two crossed polarisers, the oldest demonstration in optics — while the
// averaged model still predicts 0.00547 of the light coming back. There the
// relative error is not a factor of two, it is unbounded, and the thing this
// renderer cannot produce is the *absence* of a reflection.
//
// What it would cost to fix: four Stokes parameters per wavelength instead of
// one number, a four-by-four Mueller matrix at every interaction instead of a
// scalar multiply, and a rotation into each surface's plane of incidence on
// the way in and out — roughly four times the state on every path and a
// rewrite of every material. Item 0143 is where that is costed, under `later`,
// with the measurement attached rather than a shrug.
//
// **Thin films.** A soap bubble, an anti-reflective coating and the colour on
// an oil slick are interference between two boundaries a fraction of a
// wavelength apart. This models one boundary and adds powers rather than
// amplitudes, so it cannot produce any of them — which `spectrum.hpp` said
// under "coherence and phase" and which this is the file that would have to
// change.
//
// **Magnetic materials.** The full equations carry a permeability alongside
// the permittivity. At optical frequencies every material's relative
// permeability is 1 to within measurement, so it is dropped here, and that is
// an approximation rather than an identity.

#pragma once

#include <cmath>
#include <complex>

#include <render/si.hpp>

namespace render {

// A refractive index. Complex, always: a dielectric is the case where the
// imaginary part is zero, and giving the two different types would be the
// beginning of giving them different code.
using Index = std::complex<double>;

// Air, near enough. The real figure is 1.000293 at 589 nm and sea level, and
// the difference matters for a mirage and for nothing in this box.
inline constexpr double index_of_air = 1.0;

// The two reflectances, never one. A caller that wants an unpolarised answer
// has to average them and say so, which is the point: the averaging is an
// approximation and it should be visible at the place it is made rather than
// hidden in here. `bsdf.hpp` makes the same argument about a sample and its
// density.
struct Reflected {
    double s = 0.0;     // field perpendicular to the plane of incidence
    double p = 0.0;     // field parallel to it

    // What this renderer actually uses, because it carries scalar radiance.
    // Correct for unpolarised light; see item 0081 for what it costs.
    constexpr double unpolarised() const { return 0.5 * (s + p); }
};

// The equations above, once, for any pair of indices.
//
// `cos_theta_i` rather than the angle, because every caller has the cosine
// already — it is a dot product with the normal — and because taking an
// arccos to take a cosine again loses precision for no reason.
inline Reflected fresnel(double cos_theta_i, const Index& eta_i, const Index& eta_t) {
    // Two-sided. A ray arriving from inside the surface has a negative cosine
    // against the outward normal, and the answer is the same boundary crossed
    // the other way.
    if (cos_theta_i < 0.0) return fresnel(-cos_theta_i, eta_t, eta_i);

    const Index ratio = eta_i / eta_t;
    const double sin_squared_i = 1.0 - cos_theta_i * cos_theta_i;

    // Snell, rearranged for the cosine rather than the sine, so that the
    // square root is the only place a branch could have been. It is not one:
    // past the critical angle the argument goes negative, the root is
    // imaginary, and `|r|^2` comes out exactly 1 without anything being
    // tested.
    const Index cos_theta_t = std::sqrt(Index{1.0, 0.0} - ratio * ratio * sin_squared_i);

    // The two amplitude ratios, as numerator and denominator rather than as a
    // quotient, because what is wanted is the squared magnitude and
    // `|a/b|^2` is `|a|^2 / |b|^2`.
    //
    // That is not a micro-optimisation, though it is one — a complex division
    // costs more than two norms and a real divide. It is exactness. Under
    // total internal reflection the numerator and denominator are complex
    // conjugates, so their magnitudes are equal *bit for bit*, and the ratio
    // is 1 with no rounding at all. Dividing first and taking the norm second
    // rounds twice and returns 1.0000000000000002 at 89 degrees, which is a
    // surface that reflects more light than arrives at it.
    //
    // Measured, over 48,140 angles past the critical angle: none of them
    // comes back above 1 and the worst is 1.5 ulp short. How many are exactly
    // 1.0 is the compiler's business rather than this file's — 50% under
    // libc++ and 100% under libstdc++, because the two implement complex
    // multiplication differently — so the figure the check asserts is the
    // invariant and not the count. What it never does is exceed 1.
    //
    // And `|r|^2` is a reflectance only when the light arrives through
    // something that does not absorb. The amplitude ratio is defined for any
    // pair of indices; turning it into a ratio of energy flux needs the real
    // part of a Poynting vector, and the two agree exactly when the incident
    // index is real. A ray inside a metal is not a thing geometric optics
    // has, so this never arises here — but a sweep that asks for it gets an
    // answer above 1, which is the equation being asked a question rather
    // than the equation being wrong.
    const Index s_over = eta_i * cos_theta_i - eta_t * cos_theta_t;
    const Index s_under = eta_i * cos_theta_i + eta_t * cos_theta_t;
    const Index p_over = eta_t * cos_theta_i - eta_i * cos_theta_t;
    const Index p_under = eta_t * cos_theta_i + eta_i * cos_theta_t;

    return Reflected{std::norm(s_over) / std::norm(s_under),
                     std::norm(p_over) / std::norm(p_under)};
}

// From air, which is almost every call.
inline Reflected fresnel(double cos_theta_i, const Index& eta_t) {
    return fresnel(cos_theta_i, Index{index_of_air, 0.0}, eta_t);
}

// ── Two closed forms, which exist to be disagreed with ───────────────────
//
// Both are consequences of the function above rather than alternatives to it,
// and they are here because a derivation checked against nothing is a
// derivation nobody has checked. `./cornell verify` runs the general function
// against each of them.

// At normal incidence the two polarisations are indistinguishable — there is
// no plane of incidence to be parallel or perpendicular to — and the algebra
// collapses to something a reader can evaluate by hand:
//
//      R = |(eta_t - eta_i) / (eta_t + eta_i)|^2
//
// which for a conductor against air is the familiar
//
//      R = ((n-1)^2 + k^2) / ((n+1)^2 + k^2)
//
// and is where the reflectance of a metal is usually quoted from.
inline double normal_incidence(const Index& eta_i, const Index& eta_t) {
    return std::norm((eta_t - eta_i) / (eta_t + eta_i));
}

inline double normal_incidence(const Index& eta_t) {
    return normal_incidence(Index{index_of_air, 0.0}, eta_t);
}

// Brewster's angle, derived above. Real indices only: a conductor has no
// angle at which `r_p` vanishes, and returning a plausible number for one
// would be inventing a phenomenon.
constexpr double brewster_angle(double eta_i, double eta_t) {
    return std::atan(eta_t / eta_i);
}

// The critical angle, past which a ray going from the denser medium to the
// thinner one cannot leave at all. `eta_i > eta_t`, or there is no such
// angle and this returns a NaN rather than a number.
constexpr double critical_angle(double eta_i, double eta_t) {
    return std::asin(eta_t / eta_i);
}

// ── Where the checks are, and why they are not here ─────────────────────
//
// Every other file in this project pins its constants with `static_assert`,
// because a check the compiler runs cannot be skipped. This one cannot.
//
// Brewster's angle is an arctangent, the critical angle is an arcsine, and
// the reflectances need a complex square root — and none of `std::atan`,
// `std::asin` or `std::sqrt(std::complex)` is `constexpr` in the standard.
// libstdc++ makes some of them work anyway as an extension and libc++ does
// not, so a `static_assert` here would compile on one of this project's two
// compilers and fail on the other, which is worse than not having one.
//
// So the claims of this file are runtime checks in `./cornell verify`, where
// they are stronger than a `static_assert` could have been anyway: Brewster's
// angle is found by *searching* for the minimum of `r_p` across incidence,
// rather than by evaluating the formula the comment above derives and
// comparing it with itself.

} // namespace render
