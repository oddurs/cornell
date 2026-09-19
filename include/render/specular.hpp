// specular.hpp — a mirror, and the first surface whose distribution is not a
// function.
//
// The law of reflection is older than optics: the angle of incidence equals
// the angle of reflection, which Hero of Alexandria derived around 60 AD from
// the assumption that light takes the shortest path — the first variational
// principle in physics, seventeen centuries before Fermat generalised it to
// the *quickest* path and got refraction as well.
//
// In the local frame `basis.hpp` provides, where the normal is +z, that whole
// law is one line: flip the tangential components and keep the normal one.
// There is no sampling, no warp, no random number consumed. The direction is
// determined.
//
// ── Which breaks the three-method contract, on purpose ───────────────────
//
// A surface that scatters in exactly one direction has a density that is a
// Dirac delta — infinite on a set of measure zero, zero everywhere else. It
// is not a function, `pdf()` cannot return it, and `eval()` cannot return a
// BRDF whose integral against a cosine is finite at a point.
//
// Every renderer handles this with a convention and most of them do not write
// it down. This one does, once, in `bsdf.hpp`: `f` and `pdf` are zero, the
// estimator arrives already taken as `weight`, and a flag says so. The two
// zeros are the honest answers rather than placeholders — the probability of
// asking about the one direction that matters is nil — and they are what make
// the chi-squared test and v0.8's multiple importance sampling safe by
// default, because a caller that forgets the flag gets nothing rather than
// something plausible.
//
// ── What the weight is, and why it is not the reflectance ────────────────
//
// For a finite lobe the path loop forms `f · cos(theta) / pdf`. For this one
// the same expression has an infinity over an infinity in it, and the limit
// is exactly the reflectance at that angle:
//
//      f(wo, wi)  =  R(theta) delta(wi - mirror(wo)) / cos(theta)
//      pdf(wi)    =            delta(wi - mirror(wo))
//
// so `f · cos / pdf` is `R(theta)`, with the delta cancelling and the cosine
// cancelling the one the BRDF carries. The `1/cos` in `f` is not a fudge to
// make that work: it is what a delta BRDF has to carry for the rendering
// equation's cosine to come back out, and writing it down is the difference
// between a convention and a coincidence.
//
// So `weight` is `R(theta)` and nothing else, and the file that supplies `R`
// decides what kind of mirror this is. A flat spectrum makes a grey one. A
// spectrum that evaluates Fresnel against a table of complex refractive index
// makes gold.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Roughness. This is the perfectly flat limit, where the microfacet
// distribution is a delta too; v0.7 is where a surface becomes a landscape of
// tiny mirrors and this becomes the case where they all point the same way.
//
// Transmission. A real mirror is a dielectric or a metal boundary, and the
// light that does not reflect goes *somewhere* — into the glass, or into the
// metal as heat. This absorbs it. v0.9 gives the dielectric case its other
// half and the conductor keeps this one, because light that enters a metal in
// geometric optics does not come back.

#pragma once

#include <render/bsdf.hpp>
#include <render/spectrum.hpp>
#include <render/vec.hpp>

namespace render {

// Hero of Alexandria, in the frame where the normal is +z.
constexpr Vec3 mirror(const Vec3& wo) { return Vec3{-wo.x, -wo.y, wo.z}; }

// A perfectly flat surface whose reflectance is a spectrum.
//
// `Spectrum` answers the same question every other material's does — what is
// your value at this wavelength — so a mirror is a mirror and what it is made
// of is a separate decision, made by whoever hands it a spectrum.
template <class Spectrum>
class Specular {
public:
    constexpr explicit Specular(Spectrum reflectance) : reflectance_{reflectance} {}

    // Zero, and it means it. There is no pair of directions at which this
    // BRDF is finite, so there is no pair at which a number could be
    // returned. See the convention.
    constexpr Brdf eval(const Vec3&, const Vec3&, const Wavelengths&) const {
        return Brdf{};
    }

    constexpr SolidAngleDensity pdf(const Vec3&, const Vec3&) const {
        return SolidAngleDensity{};
    }

    // The one direction, the reflectance at it, and the flag.
    //
    // The two random numbers are not used and are not named, which is the
    // signature of a delta lobe: it consumes nothing from the sampler, so a
    // path through a mirror advances the stream by two draws it did not need.
    // That is deliberate rather than an oversight — `sampler.hpp` addresses
    // rather than dispenses, so the draws a path makes must not depend on
    // what it hits, or two paths with the same address would diverge.
    BsdfSample sample(const Vec3& wo, const Wavelengths& lambdas, double, double) const {
        BsdfSample out;
        out.wi = mirror(wo);
        out.kind = BsdfSample::Kind::Delta;

        // `weight` is R(theta) — the limit of `f · cos / pdf`, derived above.
        // The angle is `wo`'s against the normal, which in this frame is its
        // z, and it is the same for `wi` because that is what a mirror is.
        const double cos_theta = std::fabs(wo.z);
        for (int i = 0; i < spectral_samples; ++i)
            out.weight[i] = reflectance_.at(lambdas[i], cos_theta);

        return out;
    }

private:
    Spectrum reflectance_{};
};

// The simplest thing to make one of: the same reflectance at every wavelength
// and every angle. A furnace's mirror, and nothing a real surface does.
//
// It takes the cosine and ignores it, which is the shape every reflectance
// this file accepts has to have — a mirror's reflectance depends on the angle
// for any real material, and a spectrum that did not take the angle would
// make Fresnel impossible to plug in.
struct FlatReflectance {
    double value = 1.0;

    constexpr double at(double, double) const { return value; }
};

} // namespace render
