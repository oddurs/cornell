// scene.hpp — what there is, and the file that closes the two tagged unions.
//
// It is deliberately small and deliberately temporary. v0.4 replaces most of
// it: `cornell.hpp` will hold the box as a `constexpr` specification with the
// dimensions Cornell published, and the BVH replaced the loop below. What
// survives is the shape of the thing — a flat list of surfaces, each with a
// geometry, a BSDF and possibly an emission — and the two `variant`s.
//
// ── The closed sets, which `bsdf.hpp` could not declare ──────────────────
//
// A `std::variant` names its alternatives, so the file declaring it must have
// seen every one. `bsdf.hpp` must not — a contract that includes its own
// implementors is a circular dependency wearing a contract — so the closed
// set lives here, in the file that has to know what materials exist because
// it is the thing holding them.
//
// Two of them: what a surface *is*, and what it *does*. `std::visit` over
// either compiles to a jump table, and over a single alternative to a direct
// call with nothing to jump through. No vtable, no pointer chase, and no
// indirect branch in the innermost loop of the program. The set is closed and
// known at compile time — there will never be a plugin, a scene file naming a
// material class, or a shading language — so a virtual call would buy
// extensibility this project has explicitly decided not to want.
//
// The `static_assert` below is the contract being enforced rather than
// remembered: a model added to `Bsdf` without all three methods fails here,
// naming the method it is missing.
//
// ── One-sided emission, and why two-sided is a convenience ───────────────
//
// A surface emits into the hemisphere its normal points into, and nothing out
// of the other side. That is not a simplification — it is what a physical
// emitter does. A fluorescent panel has a back, the back is attached to a
// ceiling, and no light comes out of it.
//
// Two-sided emitters exist in renderers because they are convenient: an
// author who has not thought about winding order gets a light that works
// whichever way the polygon happens to face. The cost is that the light emits
// twice the power its geometry accounts for, in a direction where there is a
// physical object, and every energy comparison afterwards is off by a factor
// that depends on how much of the back face the camera can see. v1.0 compares
// against a photometric measurement; a light emitting from a surface that in
// the real box is screwed to a ceiling would make that comparison meaningless.
//
// So: one-sided, and an emitter facing the wrong way renders black, which is
// the correct and very findable failure.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Nothing, any more. This said "any acceleration at all" until v0.4 put a
// BVH behind `intersect`, and the exhaustive loop it described is still here
// as `intersect_exhaustively` — not as the implementation but as the thing
// the implementation is checked against, which is item 0058 and which
// `./cornell verify` runs on every build.
//
// Instancing, transforms, and therefore any object appearing twice. Every
// surface carries its own world-space vertices.
//
// Emission that varies over a surface or with direction. `Radiance` here is a
// constant: the same spectral radiance in every direction on the front side,
// which is a Lambertian emitter. A real fixture has a distribution, and
// `0054` is where the box's actual light gets its measured geometry.

#pragma once

#include <optional>
#include <variant>
#include <vector>

#include <render/basis.hpp>
#include <render/bvh.hpp>
#include <render/bsdf.hpp>
#include <render/cie.hpp>
#include <render/illuminant.hpp>
#include <render/lambert.hpp>
#include <render/ray.hpp>
#include <render/sphere.hpp>
#include <render/spectrum.hpp>
#include <render/triangle.hpp>

namespace render {

// What a surface is. The geometry the renderer can intersect.
using Shape = std::variant<Sphere, Triangle>;

// What a surface does to light. Two alternatives now, and they are the same
// model over different spectra rather than two models — a grey wall and a
// wall whose reflectance came off a spectrometer differ in what they are made
// of, not in how they scatter.
using GreyLambert = Lambert<Flat>;
using SpectralLambert = Lambert<cie::Illuminant>;

// The measured paint. 76 samples at 4 nm is Cornell's reflectance grid, and
// naming it here rather than in `cornell.hpp` is the price `bsdf.hpp` agreed
// to pay: a variant must name its alternatives, so the file that closes the
// set has to know the concrete spectra even when they belong to somebody
// else's data. `cornell.hpp` asserts that this number is still its own.
inline constexpr std::size_t measured_reflectance_samples = 76;
using MeasuredLambert = Lambert<Measured<measured_reflectance_samples>>;

using Bsdf = std::variant<GreyLambert, SpectralLambert, MeasuredLambert>;

static_assert(BsdfModel<GreyLambert>,
              "every alternative of Bsdf must satisfy the three-method contract");
static_assert(BsdfModel<SpectralLambert>,
              "every alternative of Bsdf must satisfy the three-method contract");
static_assert(BsdfModel<MeasuredLambert>,
              "every alternative of Bsdf must satisfy the three-method contract");

// What a surface emits. Same argument: an emitter is a spectrum and a scale,
// because a lamp's colour and its brightness are different facts about it.
//
// Four samples at 100 nm is Cornell's published emission grid for the box's
// lamp, and it is named here for the same reason the reflectance grid is.
inline constexpr std::size_t measured_emission_samples = 4;
using MeasuredEmission = Measured<measured_emission_samples>;

using Emission = std::variant<Flat, cie::Illuminant, MeasuredEmission>;

// ── Dispatch ─────────────────────────────────────────────────────────────
// Free functions rather than members, so that a model is a plain struct that
// knows nothing about the variant it ends up in.

inline BsdfSample sample(const Bsdf& bsdf, const Vec3& wo,
                         const Wavelengths& lambdas, double u, double v) {
    return std::visit([&](const auto& model) { return model.sample(wo, lambdas, u, v); }, bsdf);
}

inline Reflectance eval(const Bsdf& bsdf, const Vec3& wo, const Vec3& wi,
                        const Wavelengths& lambdas) {
    return std::visit([&](const auto& model) { return model.eval(wo, wi, lambdas); }, bsdf);
}

inline double pdf(const Bsdf& bsdf, const Vec3& wo, const Vec3& wi) {
    return std::visit([&](const auto& model) { return model.pdf(wo, wi); }, bsdf);
}

// ── Surfaces ─────────────────────────────────────────────────────────────

struct Surface {
    Shape shape;
    Bsdf bsdf;

    // A spectrum and a scale. The scale is spectral radiance in
    // W·m⁻²·sr⁻¹·m⁻¹ at the wavelength where the spectrum is 1; for a
    // tabulated illuminant normalised to 100 at 560 nm, that is a hundredth
    // of the radiance at 560 nm, which is a strange-sounding unit and the one
    // the tables come in.
    Emission emission = Flat{0.0};
    double radiance = 0.0;

    bool emits() const { return radiance > 0.0; }
};

// Everything the integrator needs to know about where a path landed.
struct Interaction {
    double t = 0.0;
    Vec3 point{};
    Unit normal{};                      // geometric, outward
    const Surface* surface = nullptr;

    // The tangent frame the BSDF works in. Built here so that the integrator
    // never constructs one from a normal it has adjusted.
    Basis frame() const { return Basis{normal}; }
};

class Scene {
public:
    void add(Surface surface) { surfaces_.push_back(std::move(surface)); }

    // Removed as dead code by the first code review, and back because it has
    // a caller: `./cornell spec` counts and measures the primitives, and
    // v0.4's BVH builds over them. The review was right at the time.
    const std::vector<Surface>& surfaces() const { return surfaces_; }

    std::size_t node_count() const { return bvh_.nodes().size(); }

    // The tree itself, for `./cornell verify`, which has to be able to ask
    // whether every primitive is still reachable through it.
    const std::vector<BvhNode>& nodes() const { return bvh_.nodes(); }

    // Build the index. Call it once, after the last `add`.
    //
    // Separate from `add` rather than incremental, because a BVH built as
    // primitives arrive is a different and much worse tree than one built
    // knowing all of them — the surface area heuristic needs the whole set to
    // choose a split.
    void finalise() {
        std::vector<Bounds> item_bounds;
        item_bounds.reserve(surfaces_.size());
        for (const Surface& surface : surfaces_) {
            Bounds b;
            std::visit([&](const auto& shape) { grow_bounds(b, shape); }, surface.shape);
            item_bounds.push_back(b);
        }
        bvh_.build(item_bounds);
    }

    // The nearest surface along the ray, if any — through the index.
    std::optional<Interaction> intersect(const Ray& ray) const {
        // A tree of one node is a tree that decided not to be one: the
        // heuristic found no split worth making, so traversing it costs a box
        // test per ray and buys nothing. It does not arise for the Cornell
        // box — that builds 25 nodes and runs 1.59x faster than exhaustive —
        // and the guard stays for scenes small enough that it might.
        if (bvh_.nodes().size() <= 1) return intersect_exhaustively(ray);

        std::optional<Interaction> nearest;
        Ray shortened = ray;

        bvh_.traverse(ray, shortened, [&](std::uint32_t index) {
            const Surface& surface = surfaces_[index];
            const auto t = std::visit(
                [&](const auto& shape) { return shape.intersect(shortened); }, surface.shape);
            if (!t) return;

            shortened.t_max = *t;
            const Vec3 point = ray.at(*t);
            const Unit normal = std::visit(
                [&](const auto& shape) { return shape.normal_at(point); }, surface.shape);
            nearest = Interaction{*t, point, normal, &surface};
        });

        return nearest;
    }

    // The same question, asked of every primitive. Kept because item 0058 is
    // the claim that the index changes only the speed of the answer, and a
    // claim like that needs something to be compared against.
    std::optional<Interaction> intersect_exhaustively(const Ray& ray) const {
        std::optional<Interaction> nearest;
        Ray shortened = ray;

        for (const Surface& surface : surfaces_) {
            const auto t = std::visit(
                [&](const auto& shape) { return shape.intersect(shortened); }, surface.shape);
            if (!t) continue;

            // Shortening the ray as hits are found is the whole of the
            // "nearest" logic: a later surface has to beat the best so far to
            // be considered at all, and `Ray::holds` already enforces that.
            shortened.t_max = *t;

            const Vec3 point = ray.at(*t);
            const Unit normal = std::visit(
                [&](const auto& shape) { return shape.normal_at(point); }, surface.shape);

            nearest = Interaction{*t, point, normal, &surface};
        }
        return nearest;
    }

private:
    static void grow_bounds(Bounds& b, const Triangle& t) {
        b.grow(t.a); b.grow(t.b); b.grow(t.c);
    }

    static void grow_bounds(Bounds& b, const Sphere& s) {
        const Vec3 r{s.radius, s.radius, s.radius};
        b.grow(s.centre - r);
        b.grow(s.centre + r);
    }

    std::vector<Surface> surfaces_;
    Bvh bvh_;
};

// What a surface sends towards `wo`, which is nothing at all from the back.
//
// `wo` points away from the surface, towards where the light is going. The
// cosine against the outward normal is therefore positive exactly when the
// viewer is on the emitting side.
inline Radiance emitted(const Interaction& hit, const Vec3& wo,
                        const Wavelengths& lambdas) {
    if (!hit.surface->emits()) return Radiance{};
    if (dot(hit.normal, wo) <= 0.0) return Radiance{};

    // The emission spectrum, evaluated at the wavelengths this path carries.
    return std::visit([&](const auto& spectrum) {
        Radiance out;
        for (int i = 0; i < spectral_samples; ++i)
            out[i] = spectrum.at(lambdas[i]) * hit.surface->radiance;
        return out;
    }, hit.surface->emission);
}

} // namespace render
