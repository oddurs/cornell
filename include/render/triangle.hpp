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
// ── What is not modelled ─────────────────────────────────────────────────
//
// Vertex normals, and therefore smooth shading. This triangle is flat and its
// normal is the geometric one; a mesh approximating a curved surface will
// look faceted. The Cornell box is made of flat panels, so nothing in this
// project needs interpolated normals until something curved arrives that is
// not a sphere.
//
// Texture coordinates. The barycentrics are computed and discarded. They are
// what a texture lookup would use, and there are no textures.
//
// Watertightness. Two triangles sharing an edge can, for a ray passing
// exactly through that edge, both report a miss — the arithmetic for the two
// is different, so `u + v ≤ 1` on one side and the corresponding test on the
// other can disagree in the last bit. Woop, Benthin and Wald's 2013
// formulation fixes it by construction. In a box made of two-triangle walls
// that is a single pixel with a black speck in it; in a dense mesh it is a
// scatter of them. It belongs with the deliberate choice in v0.4.

#pragma once

#include <optional>
#include <render/ray.hpp>
#include <render/vec.hpp>

namespace render {

struct Triangle {
    Vec3 a{};
    Vec3 b{};
    Vec3 c{};

    // Counter-clockwise seen from the front, which is the side the normal
    // points out of. Stated once here: everything that cares — one-sided
    // emission, and back-face orientation in the integrator — reads it from
    // `normal()` rather than deciding for itself.
    Unit normal() const { return normalize(cross(b - a, c - a)); }

    // The same value for every point on a flat triangle. The argument exists
    // so that this has the shape every other surface's normal query has, and
    // so the scene can ask without knowing what it is holding.
    Unit normal_at(const Vec3&) const { return normal(); }

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
