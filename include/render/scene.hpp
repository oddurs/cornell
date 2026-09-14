// scene.hpp — what there is, and the file that closes the two tagged unions.
//
// It is deliberately small and deliberately temporary. v0.4 replaces most of
// it: `cornell.hpp` will hold the box as a `constexpr` specification with the
// dimensions Cornell published, and the BVH will replace the loop below. What
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
// Any acceleration at all. `intersect` tests every surface against every ray.
// For the dozen triangles of a box that is faster than a tree would be, and
// it is written as a plain loop so that v0.4's BVH has something obviously
// correct to be checked against — item 0058 is exactly that comparison, a
// million random rays against brute force.
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
#include <render/bsdf.hpp>
#include <render/lambert.hpp>
#include <render/ray.hpp>
#include <render/sphere.hpp>
#include <render/spectrum.hpp>
#include <render/triangle.hpp>

namespace render {

// What a surface is. The geometry the renderer can intersect.
using Shape = std::variant<Sphere, Triangle>;

// What a surface does to light. One model so far; the variant is the shape of
// the decision rather than a hedge, and adding Fresnel in v0.6 is an entry in
// this list and a `static_assert` that passes.
using Bsdf = std::variant<Lambert>;

static_assert(BsdfModel<Lambert>,
              "every alternative of Bsdf must satisfy the three-method contract");

// ── Dispatch ─────────────────────────────────────────────────────────────
// Free functions rather than members, so that a model is a plain struct that
// knows nothing about the variant it ends up in.

inline BsdfSample sample(const Bsdf& bsdf, const Vec3& wo, double u, double v) {
    return std::visit([&](const auto& model) { return model.sample(wo, u, v); }, bsdf);
}

inline Reflectance eval(const Bsdf& bsdf, const Vec3& wo, const Vec3& wi) {
    return std::visit([&](const auto& model) { return model.eval(wo, wi); }, bsdf);
}

inline double pdf(const Bsdf& bsdf, const Vec3& wo, const Vec3& wi) {
    return std::visit([&](const auto& model) { return model.pdf(wo, wi); }, bsdf);
}

// ── Surfaces ─────────────────────────────────────────────────────────────

struct Surface {
    Shape shape;
    Bsdf bsdf;
    Radiance emission{};    // black unless this is a light

    bool emits() const { return !emission.is_black(); }
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

    // The nearest surface along the ray, if any.
    //
    // Every surface, every time. See above: the BVH is v0.4's, and this loop
    // is what will be used to prove it correct.
    std::optional<Interaction> intersect(const Ray& ray) const {
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
    std::vector<Surface> surfaces_;
};

// What a surface sends towards `wo`, which is nothing at all from the back.
//
// `wo` points away from the surface, towards where the light is going. The
// cosine against the outward normal is therefore positive exactly when the
// viewer is on the emitting side.
inline Radiance emitted(const Interaction& hit, const Vec3& wo) {
    if (!hit.surface->emits()) return Radiance{};
    return dot(hit.normal, wo) > 0.0 ? hit.surface->emission : Radiance{};
}

} // namespace render
