// triangle.hpp — three points, and the first surface in this project that is
// flat.
//
// It is here earlier than the roadmap puts triangles, and the reason is worth
// stating rather than leaving as an apparent jumble. v0.4 has an item called
// "Triangle intersection: choose the algorithm deliberately", which is about
// benchmarking the candidates — Möller–Trumbore, the watertight formulation,
// the precomputed-plane variants — and picking one on evidence. That is a
// real piece of work and it is not this.
//
// This exists because v0.2 needs an *area light*, an area light is the
// simplest thing that makes a soft shadow, and the simplest emitter with an
// area is a triangle. So: the obvious algorithm, written plainly, with the
// choice deferred by name. When v0.4 measures the alternatives, this is the
// baseline they are measured against.
//
// ── Möller and Trumbore, 1997 ────────────────────────────────────────────
//
// Solve for the intersection in the triangle's own coordinates rather than
// finding the plane first. A point in the triangle is
//
//      P = v0 + u·(v1 − v0) + v·(v2 − v0),    u ≥ 0, v ≥ 0, u + v ≤ 1
//
// and a point on the ray is `o + t·d`. Setting them equal gives three
// equations in three unknowns, and Cramer's rule solves it with two cross
// products and some dot products — never forming the plane equation, never
// dividing until the end, and getting the barycentric coordinates for free,
// which is what a textured or smooth-shaded triangle would need later.
//
// ── No epsilon, which took some care ─────────────────────────────────────
//
// The published version opens with `if (|det| < EPSILON) return false`, to
// catch a ray parallel to the triangle before dividing by nearly nothing. The
// previous item in this milestone deleted the last constant of that shape
// from the project and the acceptance criterion was that none remain, so this
// file does not get to add one back.
//
// It does not need one. The test that matters is `det == 0.0`, which is
// exact: it is true precisely when the ray direction is perpendicular to the
// triangle's normal to within the arithmetic, and in that case there is no
// intersection to report. A `det` that is merely *small* is a ray hitting the
// triangle at a grazing angle, which is a real hit that a tolerance would
// throw away — and throwing it away is how a tolerance puts a visible crack
// along the shared edge of two triangles in a mesh.
//
// The cost of not having the guard is that a nearly-parallel ray divides by a
// nearly-zero determinant and gets a barycentric coordinate of enormous
// magnitude, which then fails the `u < 0 || u > 1` test and is rejected on
// its merits rather than by fiat. That is the correct outcome and it is one
// comparison later.
//
// ── Two normals, and the difference is not cosmetic ──────────────────────
//
// A triangle has a *geometric* normal — the cross product of two of its
// edges, the direction the flat plane actually faces — and it may also carry
// three vertex normals, interpolated across the face to give a *shading*
// normal that varies smoothly. The second is how a coarse mesh is made to
// look curved, and it is a lie: the surface is still flat and the normal
// claims it is not.
//
// Conflating them is a classic and expensive mistake, so this file keeps them
// apart and says which is for what:
//
//      geometric_normal()   the plane's own direction. Used by
//                           `waechter.hpp` to decide which side to offset a
//                           spawned ray onto, and by anything that asks
//                           which side of the surface a direction is on.
//                           Always. A ray offset along an interpolated
//                           normal can be pushed to the wrong side of a
//                           surface it is on, which is the bug the entire
//                           self-intersection machinery exists to prevent.
//
//      shading_normal()     the interpolated one, where there is one. Used
//                           for the BSDF and the cosine, because that is
//                           what makes the lie work.
//
// Where they disagree — near a silhouette on a coarse mesh — the shading
// normal can point away from a direction the geometric normal accepts, or the
// reverse. Light that should scatter is discarded and light that should not
// is kept, and the usual symptom is a dark rim on curved objects that nobody
// can account for. This project's box is flat panels, so the two agree
// everywhere in it, and the accessors exist so that the day something curved
// arrives the distinction is already made rather than being retrofitted.
//
// Which means, plainly: nothing in this repository sets `na`, `nb` or `nc`,
// and `shading_normal` returns the geometric normal on every ray it has ever
// been asked about. The branch is kept rather than deleted because the
// distinction is the thing being explained, and it is worth a dozen lines to
// have it explained where it applies; the day a curved surface arrives, this
// paragraph is what has to change and not the design.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Texture coordinates. The barycentrics are computed, used for the shading
// normal, and otherwise discarded. They are what a texture lookup would use,
// and there are no textures.
//
// Watertightness. Two triangles sharing an edge can, for a ray passing
// exactly through that edge, both report a miss — the arithmetic for the two
// is different, so `u + v ≤ 1` on one side and the corresponding test on the
// other can disagree in the last bit. Woop, Benthin and Wald's 2013
// formulation fixes it by construction.
//
// v0.4 measured it rather than inheriting the worry, and kept this algorithm.
// Item 0055 has the figures; the short version is that a ray aimed *exactly*
// at a shared edge misses 26.6 % of the time, and that 3.4 million rays fired
// at a sealed box — including 1.4 million on an adversarial grid of exact
// fractions, against axis-aligned geometry — escaped zero times.
//
// The difference is `double`. Woop's leak rates are `float` leak rates, and
// 52 bits of mantissa move this failure from occasionally visible to only
// when aimed at on purpose. If this project ever moves to `float`, or gets
// meshes dense enough that one pixel spans several shared edges, the answer
// changes and the test is written down so it can be re-run.

#pragma once

#include <optional>
#include <utility>
#include <render/ray.hpp>
#include <render/vec.hpp>

namespace render {

struct Triangle {
    Vec3 a{};
    Vec3 b{};
    Vec3 c{};

    // Optional vertex normals. All three zero means flat, which is what a
    // default-constructed `Unit` gives and what the Cornell box wants.
    Unit na{};
    Unit nb{};
    Unit nc{};

    // Counter-clockwise seen from the front, which is the side the normal
    // points out of. Stated once here: everything that cares — one-sided
    // emission, the offset in `waechter.hpp`, back-face orientation in the
    // integrator — reads it from here rather than deciding for itself.
    Unit geometric_normal() const { return normalize(cross(b - a, c - a)); }

    bool smooth() const {
        return length_squared(na) > 0.0 || length_squared(nb) > 0.0 || length_squared(nc) > 0.0;
    }

    // The interpolated normal at a point on the face, or the geometric one if
    // this triangle has no vertex normals. See the header for which consumer
    // wants which.
    Unit shading_normal(const Vec3& p) const {
        if (!smooth()) return geometric_normal();

        const auto [u, v] = barycentric(p);
        const Vec3 blended = na.vec() * (1.0 - u - v) + nb.vec() * u + nc.vec() * v;
        return length_squared(blended) > 0.0 ? normalize(blended) : geometric_normal();
    }

    // Where a point on the plane sits in the triangle's own coordinates.
    // Recovered from the point rather than returned by `intersect`, so that
    // the shape interface every primitive shares stays one number wide.
    std::pair<double, double> barycentric(const Vec3& p) const {
        const Vec3 e1 = b - a, e2 = c - a, to_p = p - a;
        const double d11 = dot(e1, e1), d12 = dot(e1, e2), d22 = dot(e2, e2);
        const double dp1 = dot(to_p, e1), dp2 = dot(to_p, e2);
        const double denominator = d11 * d22 - d12 * d12;
        if (denominator == 0.0) return {0.0, 0.0};   // degenerate
        return {(d22 * dp1 - d12 * dp2) / denominator,
                (d11 * dp2 - d12 * dp1) / denominator};
    }

    // What the scene asks for when it does not know what it is holding.
    // Shading, because that is what the BSDF wants; anything that needs the
    // plane's own direction asks for it by name.
    Unit normal_at(const Vec3& p) const { return shading_normal(p); }

    std::optional<double> intersect(const Ray& ray) const {
        const Vec3 e1 = b - a;
        const Vec3 e2 = c - a;

        const Vec3 p = cross(ray.direction, e2);
        const double determinant = dot(e1, p);

        // Exactly zero, not nearly zero. See the header: a small determinant
        // is a grazing hit, which is a hit.
        if (determinant == 0.0) return std::nullopt;

        const double inverse = 1.0 / determinant;

        const Vec3 to_a = ray.origin - a;
        const double u = dot(to_a, p) * inverse;
        if (u < 0.0 || u > 1.0) return std::nullopt;

        const Vec3 q = cross(to_a, e1);
        const double v = dot(ray.direction, q) * inverse;
        if (v < 0.0 || u + v > 1.0) return std::nullopt;

        const double t = dot(e2, q) * inverse;
        return ray.holds(t) ? std::optional<double>{t} : std::nullopt;
    }
};

} // namespace render
