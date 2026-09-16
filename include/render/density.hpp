// density.hpp — a probability density over solid angle, which is not a
// number.
//
// This file exists because of a division that appears once in this project
// and carries the whole Monte Carlo argument:
//
//      throughput *= f · cos(theta) / pdf
//
// `f` is a BRDF, per steradian. `pdf` is a density over directions, per
// steradian. The quotient is dimensionless, which is why a throughput is a
// fraction rather than a quantity with units — and until this file existed,
// none of that was anywhere the compiler could see it. Both were `double`,
// and the cancellation was a fact about the author's attention.
//
// ── There is more than one density in this program ───────────────────────
//
// Which is the reason the type says which one it is, rather than being called
// `Density` and leaving it to context. `spectrum.hpp` has a second one:
// `Wavelengths::pdf()` is the density of the hero wavelength draw, and it is
// per *metre of wavelength*, not per steradian. Two densities, both `double`,
// differing by sr·m⁻¹, and both spelled `pdf` at their call sites.
//
// Nothing has yet divided one into the other. `cie.hpp` divides by the second
// and `transport.hpp` by the first, in different files, and the reason that
// has stayed correct is that no expression has ever needed both — which is
// not a safeguard, it is a coincidence with a deadline. v0.8's multiple
// importance sampling combines densities from strategies that were not
// written together, and that is the milestone where a coincidence like this
// one stops holding.
//
// ── What it deliberately does not do ─────────────────────────────────────
//
// It does not add. Two densities over the same solid angle *can* be added —
// that is exactly what a mixture density is, and v0.8 will want one — but
// adding densities over *different* measures is the mistake this type exists
// to catch, and nothing needs the sum yet. It arrives with the file that
// needs it, which is the only way to know which of the two it should be.
//
// It does not convert to `double` implicitly. `per_steradian()` is spelled
// out and is a sentence long at the call site, which is the correct price for
// leaving the type system.
//
// It is not a full dimensional-analysis system. There is no `Quantity<L, M,
// T>` here and there will not be one: the project has exactly one unit
// confusion worth machinery — the steradian, which is what separates radiance
// from irradiance and a BRDF from a reflectance — and si.hpp's opening says
// the rest is handled by refusing to let non-SI units in at all.

#pragma once

namespace render {

// A probability density with respect to solid angle. sr⁻¹.
//
// Default-constructs to zero, which is the honest density of a direction that
// was not drawn: an impossible event has no density, and a sample carrying
// this one is a sample the estimator must discard rather than divide by.
class SolidAngleDensity {
public:
    constexpr SolidAngleDensity() = default;
    constexpr explicit SolidAngleDensity(double per_steradian) : v_{per_steradian} {}

    // The way out, named for its unit so that a reader of the call site knows
    // which density escaped. `./cornell chi2` in v0.5 is the caller that has
    // to have it: comparing a density against a histogram of counts means
    // integrating it over a bin, and an integral is arithmetic on doubles.
    constexpr double per_steradian() const { return v_; }

    // Whether there is anything to divide by. Spelled as a question rather
    // than left as `pdf > 0`, because the comparison is the one place a
    // density gets confused with the value it is a density of.
    constexpr bool positive() const { return v_ > 0.0; }

private:
    double v_ = 0.0;
};

} // namespace render
