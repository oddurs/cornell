// spectrum.hpp — light, as a function of wavelength, which is what it is.
//
// This file is the one that makes the rest of the project possible, and it is
// the one that costs something on every line of every other file. The whole
// argument for paying that cost is short:
//
//      A colour is not a property of light. It is a property of an observer.
//
// What light has is a spectral power distribution — how much energy at each
// wavelength — and that is a function, not three numbers. Three numbers are
// what is left after a human retina has thrown almost all of it away. A
// renderer that starts with the three numbers has thrown it away too, before
// doing any physics, and can never get it back: it cannot disperse light
// through a prism, it cannot compute the colour of gold from measured optical
// constants, and it cannot answer what a red wall looks like under a sodium
// lamp except by inventing something.
//
// So: no RGB anywhere in this project except `cie.hpp` and `srgb.hpp`, which
// arrive in v0.3 and say loudly what they are doing. Everything upstream of
// them is spectra.
//
// ── How a spectrum is carried ─────────────────────────────────────────────
//
// Not as a curve. A path through the scene carries FOUR wavelengths, drawn
// together, and that choice is the answer to the spike that opened this
// milestone. The alternatives were:
//
//      Fixed uniform bins, say 32 of them across the visible range. Simple,
//      and it bands. A prism in v0.9 would produce thirty-two coloured
//      stripes rather than a spectrum, and no sample count fixes it, because
//      the banding is in the representation rather than in the noise.
//
//      One wavelength per path. Correct, unbiased, and extremely noisy: every
//      path throws away three quarters of the colour information it could
//      have carried for almost nothing.
//
//      Four, as hero wavelength sampling (Wilkie et al., 2014). Draw one
//      wavelength uniformly, derive the other three by rotating a quarter of
//      the visible range at a time, and carry all four along the same path.
//      Where nothing disperses — which is almost everywhere — all four stay
//      together and the path costs one intersection for four wavelengths of
//      information. Where something does disperse, they separate, and the
//      multiple importance sampling machinery from v0.8 combines them.
//
// Four is the answer. The rotation is what makes it work: four wavelengths
// drawn independently would clump, and four spread exactly a quarter of the
// range apart are stratified for free.
//
// ── What is not modelled ──────────────────────────────────────────────────
//
// Polarisation. A complete description of light at a wavelength is four
// Stokes parameters, and this file carries one number. That is correct for
// unpolarised light and progressively wrong for light that has reflected at a
// steep angle. See `fresnel.hpp` in v0.6, which computes both polarisations
// and then averages them, which is where the information is actually lost.
//
// Coherence and phase. Two beams at the same wavelength here add their
// powers; real ones add their amplitudes and can cancel. That is why this
// project cannot render a soap film, an oil slick, or the anti-reflective
// coating on its own lenses, and it is the assumption named at the top of
// `transport.hpp` rather than discovered later.

#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <render/si.hpp>

namespace render {

// How many wavelengths a path carries. Four is not arbitrary — see above —
// but nothing below hard-codes it, so that the cost of the decision can be
// measured rather than argued about.
inline constexpr int spectral_samples = 4;

// ── The wavelengths a particular path is carrying ─────────────────────────
//
// Not a property of the light; a property of the *path*. Two paths through
// the same scene carry different wavelengths, which is the entire point: over
// many paths the whole visible range gets sampled, and each individual path
// stays cheap.
class Wavelengths {
public:
    // Draw a set from one uniform random number in [0, 1).
    //
    // The hero wavelength is placed by `u`; the other three follow at even
    // quarters of the visible range, wrapping. So the four are stratified by
    // construction and the whole set is determined by a single sample, which
    // is what lets the caller stratify *across paths* as well by stratifying
    // `u` — a nicety that costs nothing and will matter in v0.8.
    static Wavelengths sample(double u) {
        Wavelengths w;
        constexpr double span = si::lambda_max - si::lambda_min;
        for (int i = 0; i < spectral_samples; ++i) {
            const double offset = u + double(i) / double(spectral_samples);
            w.lambda_[std::size_t(i)] = si::lambda_min + span * (offset - std::floor(offset));
        }
        return w;
    }

    // Every wavelength is drawn uniformly over the visible range, so they all
    // share one density, and it is a density *per metre of wavelength* —
    // which is a strange-looking unit and the correct one.
    //
    // Nothing calls this yet, and it stays anyway, which needs saying because
    // this project does not keep dead code outside `si.hpp`. House rule 3 is
    // that a sample and the density it was drawn from are one object: this is
    // the density of the draw `sample()` makes, and `cie.hpp` in v0.3 divides
    // by it the moment the film starts integrating against the observer. A
    // `sample` whose density is unreachable is the thing `bsdf.hpp` spends a
    // page refusing.
    static constexpr double pdf() {
        return 1.0 / (si::lambda_max - si::lambda_min);
    }

    constexpr double operator[](int i) const { return lambda_[std::size_t(i)]; }

    // True once a path has refracted dispersively and the four wavelengths no
    // longer travel together. Nothing sets it yet; v0.9 does, and the flag is
    // here so that the file which introduces dispersion changes one line
    // rather than the shape of every signature above it.
    constexpr bool separated() const { return separated_; }
    constexpr void separate()        { separated_ = true; }

private:
    std::array<double, spectral_samples> lambda_{};
    bool separated_ = false;
};

// ── Quantities carried at those wavelengths ───────────────────────────────
//
// Four numbers, and a tag that says what they are. The tag is the whole
// reason this is a template: radiance and reflectance are both four doubles
// and they are not the same thing, they do not add to each other, and the
// compiler should say so rather than let a plausible-looking expression
// compile into a wrong image.
//
// This is `windsor`'s `Bore` and `Stroke` argument — a bore is not a stroke
// and you should not be able to swap them by accident — applied to the
// confusion that is actually endemic in this field. Radiance, irradiance,
// radiosity and intensity differ by steradians and square metres, they are
// spelled `L`, `E`, `B` and `I`, and mixing them up is the oldest mistake in
// radiometry.
//
// It compiles to four doubles. `Tag` has no members and is never instantiated.

template <class Tag>
class Sampled {
public:
    constexpr Sampled() = default;
    constexpr explicit Sampled(double v) { v_.fill(v); }

    // The index is an `int` in the signature and a `size_t` at the array,
    // spelled out rather than left implicit: every loop over wavelengths in
    // this project counts with an `int`, `std::array` subscripts with an
    // unsigned, and `-Wsign-conversion` is right that the boundary between
    // them is where an index bug would live. Writing the conversion is how
    // the warning stays worth reading.
    //
    // These two are the *only* place that conversion happens. Every loop in
    // this file, including the private ones, goes through them rather than
    // touching `v_` directly — which is not style. The members below were
    // written against the array, the warning could not see them because
    // nothing had instantiated them yet, and they surfaced one at a time over
    // three commits as each was first called. One crossing, checked once.
    constexpr double  operator[](int i) const { return v_[std::size_t(i)]; }
    constexpr double& operator[](int i)       { return v_[std::size_t(i)]; }

    constexpr Sampled& operator+=(const Sampled& o) {
        for (int i = 0; i < spectral_samples; ++i) (*this)[i] += o[i];
        return *this;
    }
    constexpr Sampled& operator*=(double s) {
        for (int i = 0; i < spectral_samples; ++i) (*this)[i] *= s;
        return *this;
    }

    friend constexpr Sampled operator+(Sampled a, const Sampled& b) { return a += b; }
    friend constexpr Sampled operator*(Sampled a, double s)         { return a *= s; }
    friend constexpr Sampled operator*(double s, Sampled a)         { return a *= s; }

    constexpr bool is_black() const {
        for (int i = 0; i < spectral_samples; ++i) if ((*this)[i] != 0.0) return false;
        return true;
    }

    // Deliberately absent: a conversion to double, an average, a luminance, a
    // `.rgb()`. Every one of them would be a place where a spectrum quietly
    // stops being a spectrum, and there is exactly one legitimate way out of
    // this type — through the standard observer in `cie.hpp` — which does not
    // exist yet and which nothing here should pre-empt.

private:
    std::array<double, spectral_samples> v_{};
};

// ── What a material is made of ───────────────────────────────────────────
//
// A spectrum, in the sense the materials use, is anything that can answer one
// question: what is your value at this wavelength. A flat grey answers with a
// constant, a measured wall answers from a table, a fitted upsample of an RGB
// answers from three coefficients.
//
// It is a concept rather than a base class for the usual reason — there is no
// dispatch to pay for, the set is closed, and a material that holds one is a
// template over it, which compiles to the arithmetic and nothing else.
template <class T>
concept SpectralValue = requires(const T& s, double lambda) {
    { s.at(lambda) } -> std::same_as<double>;
};

// The simplest one, and the only one this project needed for two milestones:
// the same value at every wavelength. A grey wall, and an honest description
// of one — real grey paint is not flat, which is item 0053's problem.
struct Flat {
    double value = 0.0;

    constexpr double at(double) const { return value; }
};

static_assert(SpectralValue<Flat>);

// A spectrum somebody measured: a first wavelength, a step, and the samples.
//
// Every measured data set this project will use comes on its own grid —
// Cornell's walls at 4 nm from 400 to 700, the CIE observer at 5 nm from 360
// to 830, Johnson and Christy's metals at intervals that are not even uniform
// — so the grid travels with the numbers rather than being a global
// convention that half the data does not obey.
//
// Outside the measured range it returns the nearest end rather than zero.
// That is a choice and it is the less wrong one: a wall does not stop
// reflecting at 701 nm, and this project samples wavelengths out to 830 where
// the observer is not quite zero. Reporting 0 there would darken every
// surface by however much of the observer lies outside the data, which is a
// systematic error; holding the endpoint is an extrapolation, which is an
// admission. `cornell.hpp` says how much of the observer that is.
template <std::size_t N>
struct Measured {
    double first = 0.0;
    double step = 0.0;
    std::array<double, N> table{};

    constexpr double at(double lambda) const {
        const double last = first + step * double(N - 1);
        if (lambda <= first) return table[0];
        if (lambda >= last)  return table[N - 1];

        const double position = (lambda - first) / step;
        const std::size_t index = std::size_t(position);
        const double fraction = position - double(index);
        return table[index] * (1.0 - fraction) + table[index + 1] * fraction;
    }

    constexpr double lowest() const {
        double m = table[0];
        for (std::size_t i = 1; i < N; ++i) m = table[i] < m ? table[i] : m;
        return m;
    }

    constexpr double highest() const {
        double m = table[0];
        for (std::size_t i = 1; i < N; ++i) m = table[i] > m ? table[i] : m;
        return m;
    }
};

static_assert(SpectralValue<Measured<2>>);

struct RadianceTag;     // L, W·m⁻²·sr⁻¹·m⁻¹  — what a path carries
struct ReflectanceTag;  // unitless in [0,1]  — what a surface keeps

using Radiance    = Sampled<RadianceTag>;
using Reflectance = Sampled<ReflectanceTag>;

// Light meeting a surface. This is the only cross-type operation there is,
// and it is the only one that means anything: a reflectance scales a
// radiance and the result is a radiance. Two radiances do not multiply. A
// reflectance and a radiance do not add.
constexpr Radiance operator*(const Reflectance& r, const Radiance& l) {
    Radiance out;
    for (int i = 0; i < spectral_samples; ++i) out[i] = r[i] * l[i];
    return out;
}
constexpr Radiance operator*(const Radiance& l, const Reflectance& r) { return r * l; }

// Reflectances compose — light through two filters, or two bounces — and the
// result is still a reflectance, which is why this one is worth spelling out
// separately rather than falling out of a generic operator.
constexpr Reflectance operator*(const Reflectance& a, const Reflectance& b) {
    Reflectance out;
    for (int i = 0; i < spectral_samples; ++i) out[i] = a[i] * b[i];
    return out;
}

} // namespace render
