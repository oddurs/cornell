// conductor.hpp — where the colour of a metal comes from, given the numbers.
//
// This is the join. `fresnel.hpp` has the equations and takes a complex index
// at one wavelength. `specular.hpp` has a mirror and takes a reflectance that
// varies with wavelength and angle. A metal is the second asking the first,
// once per wavelength, per bounce:
//
//      R(lambda, theta) = fresnel(cos theta, eta(lambda)).unpolarised()
//
// and that line is the whole of the project's central claim. There is no
// colour in it. There is no tint, no multiplier, no artist. There is a table
// of `eta = n + ik` indexed by wavelength, and whatever falls out of Fresnel
// is what the metal looks like.
//
// Swap the table and the colour changes because the equations say so. That is
// either true or it is not, and `./cornell verify` is where it is asked.
//
// ── Why the reflectance has to depend on both ────────────────────────────
//
// Every other material in this project answers one question — what is your
// value at this wavelength. A metal answers two, because Fresnel is a
// function of angle as well, and the angle dependence is not a detail: a
// metal at grazing incidence is a *better* mirror than head on, and its
// colour washes out towards white as it goes, because the reflectance of
// every wavelength converges on 1.
//
// So the spectrum this file produces takes `(lambda, cos_theta)`, which is
// the shape `specular.hpp` already asks for, and which is why that file asks
// for it: it was written knowing this one was coming.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// **That n and k depend on the sample.** Johnson and Christy measured
// evaporated films. A rolled sheet, an electroplated surface, a cast ingot
// and a sputtered coating differ, and the differences between them are larger
// than the error on any one measurement. A renderer quoting one table is
// rendering one piece of metal somebody made in 1972.
//
// **Oxidation.** Real aluminium has a few nanometres of oxide on it within
// seconds of meeting air, real silver tarnishes, and both are thin films —
// interference, which is a wave effect this model cannot produce at all. The
// tables are for clean surfaces that do not exist outside a vacuum chamber.
//
// **Temperature.** n and k drift with it, which matters for a filament and
// not for a doorknob.
//
// **Roughness**, which is v0.7 and is the thing that makes a metal look like
// an object rather than a hole into another room.

#pragma once

#include <render/fresnel.hpp>
#include <render/specular.hpp>
#include <render/spectrum.hpp>

namespace render {

// Something that answers: what is your complex refractive index at this
// wavelength. The same shape as `SpectralValue`, returning an `Index` rather
// than a double, because a metal's index is two numbers and a dielectric's is
// the case where the second is zero.
template <class T>
concept ComplexSpectralValue = requires(const T& s, double lambda) {
    { s.at(lambda) } -> std::same_as<Index>;
};

// The simplest one: the same index everywhere. Not a metal — no real material
// has a flat complex index across the visible, and one that did would be grey
// — and it is what the checks use, because a constant makes the *machinery*
// testable without a table to be wrong about.
struct FlatIndex {
    Index value{1.0, 0.0};

    constexpr Index at(double) const { return value; }
};

static_assert(ComplexSpectralValue<FlatIndex>);

// A metal's reflectance: Fresnel, evaluated at the index this wavelength has.
//
// This is the type `specular.hpp` takes. Together they are a mirror made of
// whatever the table says, and nothing in either file knows what a colour is.
template <class IndexSpectrum>
class ConductorReflectance {
public:
    constexpr explicit ConductorReflectance(IndexSpectrum index) : index_{index} {}

    // Unpolarised, which is the approximation this renderer makes and the one
    // `fresnel.hpp` quantifies: exact for light meeting a surface for the
    // first time, out by a factor of two for two bounces at Brewster's angle,
    // and unbounded for two crossed ones. The averaging is here, at the call
    // site, rather than inside the equations, so that the place it happens is
    // the place it is admitted.
    double at(double lambda, double cos_theta) const {
        return fresnel(cos_theta, index_.at(lambda)).unpolarised();
    }

private:
    IndexSpectrum index_{};
};

// And the mirror itself, which is the two files joined.
template <class IndexSpectrum>
using Conductor = Specular<ConductorReflectance<IndexSpectrum>>;

} // namespace render
