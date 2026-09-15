// bsdf.hpp — the contract every surface in this project signs.
//
// A BSDF answers one question: light arrives from there and leaves towards
// here — what fraction, and in what direction does it prefer to go? It is the
// `f` in the rendering equation, it is the only thing that distinguishes
// plaster from gold, and the shape of its interface is the most consequential
// decision in this project after the spectral one.
//
// Three methods, and the third is the argument.
//
//      sample(wo, u, v) -> { wi, f, pdf }    draw a direction, and say how
//                                            likely that draw was
//      eval(wo, wi)     -> f                 what this surface does to light
//                                            going from wi to wo
//      pdf(wo, wi)      -> density           how likely `sample` would have
//                                            been to produce this wi
//
// ── Why `pdf` is not redundant ───────────────────────────────────────────
//
// It looks redundant. `sample` already returned a density; why would anybody
// ask for it again, for a direction they already have?
//
// Because in v0.8 the direction will not have come from here. Veach's
// multiple importance sampling combines two strategies that are each terrible
// where the other is excellent — sampling the BSDF, which is right for a
// mirror lit by a large dim room, and sampling the light, which is right for
// a matte floor under a small bright lamp — and weights each sample by how
// likely *the other* strategy would have been to find it. That weight is
// literally `pdf_bsdf(wi)` evaluated on a direction the light sampler chose.
//
// A renderer that returns the density only from `sample` cannot compute it.
// The information exists — it is implied by the sampling routine — but it is
// not *reachable*, and the fix is not a refactor: every material in the
// project has to be reopened and told how to answer a question it was never
// designed to answer. Projects discover this about two months in.
//
// So the third method is here from the first material, when there is exactly
// one BSDF and nothing to combine, and it costs a function nobody calls yet.
//
// The second thing it buys arrives sooner. `sample` and `pdf` are two
// independent claims about the same distribution, written in different code,
// and a machine can check that they agree: draw a few million directions with
// one, histogram them, and compare against the other integrated over each
// bin. That is `./cornell chi2` in v0.5, and it is only expressible because
// the two are separate. A sampler that returns its own density is a witness
// testifying to its own honesty.
//
// ── Why a variant and not a virtual ──────────────────────────────────────
//
// The set of BSDFs in this project is closed and known when it compiles.
// There will never be a plugin, a scene file that names a material class, or
// a shading language. Given that, a virtual call buys nothing and costs an
// indirect branch and a pointer chase in the innermost loop of the program —
// and `windsor`'s rule applies: a tagged union is what you would have written
// by hand for a tagged union, so that is what gets written.
//
// `std::visit` over a `std::variant` compiles to a jump table, and with a
// single alternative it compiles to a direct call with no table at all.
//
// ── Where the closed set lives, and why it is not here ───────────────────
//
// A `std::variant` has to name its alternatives, so whatever file declares it
// has to have seen every model. This file must not: a contract that includes
// its implementors is not a contract, it is a circular dependency wearing
// one, and `lambert.hpp` including `bsdf.hpp` while `bsdf.hpp` includes
// `lambert.hpp` works only until somebody includes them in the other order.
//
// So the contract is here and the closed set is declared in `scene.hpp`,
// which is the file that has to know what materials exist anyway, because it
// is the thing holding them. The concept below is what makes that safe: a
// model that does not satisfy all three methods fails to compile at the point
// it is added to the set, with an error that names the method it is missing.
//
// ── Every method takes the wavelengths, and that is the v0.3 change ──────
//
// A reflectance is a function of wavelength. `Reflectance` has been able to
// hold one since v0.1 — it is four numbers — but the four wavelengths differ
// from path to path, so a surface cannot say what it reflects without being
// told which wavelengths it is being asked about.
//
// For two milestones nothing needed to ask. Every material was grey, every
// emitter was flat, and the four components of every `Reflectance` were the
// same number, so the question never came up. It comes up the moment a wall
// is red, and a red wall is the object this project exists to render.
//
// So all three methods take a `Wavelengths`. It is the change `box.hpp`
// predicted when it explained why its walls were grey and refused to type a
// plausible red two milestones early, and it is why that refusal cost
// nothing: the walls were always going to arrive as spectra, and this is the
// signature they arrive through.
//
// The alternative — a material that carries a fixed `Reflectance` chosen when
// the scene was built — is what a renderer does when it starts in RGB, and it
// cannot represent a measured spectrum at all.
//
// ── Conventions, stated once ─────────────────────────────────────────────
//
// Both directions are in the local frame from `basis.hpp`, where the surface
// normal is +z. Both point *away* from the surface: `wo` towards the eye,
// `wi` towards where the light came from. That is the physicists' convention
// rather than the "direction of travel" one, and it is chosen because it
// makes the BSDF's reciprocity — `f(wo, wi) == f(wi, wo)` — visible as a
// symmetry in the code rather than a fact you have to remember.
//
// ── What the type system is not tracking, and should be ──────────────────
//
// `f` is returned as a `Reflectance`, and a BRDF is not a reflectance. A
// reflectance is dimensionless and at most 1; a BRDF has units of inverse
// steradians and is unbounded — a mirror's is a delta function. The estimator
// `f · cos(theta) / pdf` is dimensionless because the sr⁻¹ in `f` cancels the
// sr⁻¹ in `pdf`, and none of that is in the types.
//
// This is the one place the project's own argument — that `Radiance` and
// `Irradiance` are distinct types because they differ by a steradian — is not
// being enforced, and saying so is better than letting a reader assume it was
// considered. Fixing it needs `Sampled` to carry units rather than a bare tag,
// which is a change to `spectrum.hpp` and to every expression downstream of
// it. It is filed as its own item rather than smuggled in here.

#pragma once

#include <cmath>
#include <concepts>
#include <render/spectrum.hpp>
#include <render/vec.hpp>

namespace render {

// A direction, what the surface does along it, and the density it was drawn
// with. All three, always: house rule 3 is that a sample and its density are
// one object, because a caller holding only the first has already lost.
struct BsdfSample {
    Vec3 wi{};                  // local frame, pointing away from the surface
    Reflectance f{};            // the BRDF value, per steradian — see above
    double pdf = 0.0;           // per steradian

    // A sample that carries no light. Returned rather than an empty optional
    // because "the surface absorbed it" is an outcome the path loop handles
    // the same way it handles everything else: multiply by zero and stop.
    constexpr bool is_black() const { return pdf <= 0.0 || f.is_black(); }
};

// The contract, as something the compiler checks.
//
// The criterion for this item was "no BSDF may be added without all three
// methods". A concept is how that stops being a rule somebody remembers: a
// model missing `pdf` does not fail at the call site months later, it fails
// where it is declared, and the diagnostic names the method.
template <class T>
concept BsdfModel = requires(const T& bsdf, Vec3 wo, Vec3 wi, double u,
                             const Wavelengths& lambdas) {
    { bsdf.sample(wo, lambdas, u, u) } -> std::same_as<BsdfSample>;
    { bsdf.eval(wo, wi, lambdas) }     -> std::same_as<Reflectance>;
    { bsdf.pdf(wo, wi) }               -> std::same_as<double>;
};

// `pdf` is the exception, and the asymmetry is worth a sentence. A density
// over directions does not depend on wavelength: this project samples a
// direction and carries four wavelengths along it, rather than sampling a
// direction per wavelength. The day something disperses — v0.9's prism — the
// path splits instead, which `Wavelengths::separated()` has been waiting for
// since v0.1.

// Whether two directions are on the same side of the surface. A BSDF that
// only reflects returns nothing when they are not, and the test is spelled
// once here rather than three times in every model — where one of the three
// will eventually be written with the wrong comparison and produce light
// leaking through a wall.
constexpr bool same_hemisphere(const Vec3& a, const Vec3& b) {
    return a.z * b.z > 0.0;
}

// The cosine every shading calculation actually wants.
//
// In the local frame the normal is +z, so the cosine of a direction against
// it is that direction's z and nothing more. It is spelled out here because
// it was previously a helper in `basis.hpp` taking world-space arguments,
// which is not the space any caller is in — so both callers ignored it and
// wrote `std::fabs(w.z)` by hand, and the helper sat unused next to a comment
// claiming it existed to avoid exactly that.
//
// The absolute value is the two-sided part: a surface hit from behind has a
// negative z, and what the estimator wants is the foreshortening, which does
// not care which side it was approached from.
inline double abs_cos_theta(const Vec3& w) { return std::fabs(w.z); }

} // namespace render
