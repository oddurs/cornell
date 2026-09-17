// enclosure.hpp — a closed box, and the two questions it answers exactly.
//
// Two instruments in this project need a scene whose answer is known before
// the renderer is asked. `furnace.hpp` needs a uniform environment so that an
// energy-conserving surface can vanish in it. `converge.hpp` needs an integral
// with a closed form, so that an error can be measured against something
// other than a longer run of the same code.
//
// Both are the same object — six walls facing inward — and it is one file
// rather than two copies, because two copies of a cube is two places for a
// winding order to be wrong in only one of them.
//
// ── Why an enclosure at all ──────────────────────────────────────────────
//
// "A uniform environment of radiance 1" is usually an infinite constant
// environment map, and this project has none: a ray that escapes finds
// nothing, and `transport.hpp` says so rather than quietly returning black.
// Walls that emit radiance 1 and reflect nothing *are* that environment, seen
// from inside, and they are built out of things the scene already has.
//
// Which matters for more than convenience. An instrument here may not reach
// into the physics to make its own job easier, so a scene made of the same
// `Surface`, the same `Lambert` and the same BVH as the Cornell box is the
// only kind of scene these tests are allowed to have.
//
// ── The two answers ──────────────────────────────────────────────────────
//
// **Walls that emit and do not reflect.** Every wall carries exactly the
// radiance it emits, because nothing accumulates. A ray from anywhere inside
// returns `Le`, and an object placed in the middle is lit by a uniform
// environment. This is the furnace.
//
// **Walls that emit and reflect.** The radiance inside is isotropic — every
// direction looks like every other — so it satisfies its own equation:
//
//      L = Le + rho * L        and therefore        L = Le / (1 - rho)
//
// which is a geometric series that a renderer has to sum the hard way, one
// bounce at a time, and which is exact for the reader. There is no reference
// image and no tolerance chosen afterwards; there is a number.
//
// The second is the one that makes a convergence measurement mean something.
// A slope fitted against a long render of the same scene by the same code
// measures how fast the noise falls and is blind to anything both runs get
// wrong together; a slope fitted against `Le / (1 - rho)` is not.
//
// ── What it is not ───────────────────────────────────────────────────────
//
// It is not a Cornell box and it is not trying to be. It has no light
// source, in the sense of a small bright thing — every surface is the light —
// so it exercises none of the sampling that makes a real scene hard, and a
// result here is a result about the estimator rather than about the renderer.
// That is the point: a test scene with a closed form has to be simple enough
// to have one.

#pragma once

#include <render/lambert.hpp>
#include <render/scene.hpp>
#include <render/vec.hpp>

namespace app {

namespace enclosure {

using namespace render;

// Two triangles, wound so the normal comes out facing `inward`. Emission is
// one-sided — `scene.hpp` returns nothing from the back of a light — so a
// wall facing the wrong way is a wall that is not there.
inline void add_wall(Scene& scene, const Vec3& a, const Vec3& b, const Vec3& c,
                     const Vec3& d, const Vec3& inward, const Bsdf& bsdf,
                     const Emission& emission, double radiance) {
    Triangle first{a, b, c};
    Triangle second{a, c, d};
    if (dot(first.geometric_normal(), inward) < 0.0) {
        first  = Triangle{a, c, b};
        second = Triangle{a, d, c};
    }
    scene.add(Surface{first,  bsdf, emission, radiance});
    scene.add(Surface{second, bsdf, emission, radiance});
}

// Six of them, facing in. `half` is the half-extent, and it is large enough
// that a camera and an object fit inside with room to spare; nothing in
// either instrument depends on the size, which `verify.hpp`'s scale check is
// the general statement of.
inline void add_box(Scene& scene, double half, const Bsdf& bsdf,
                    const Emission& emission, double radiance) {
    const double lo = -half, hi = half;

    add_wall(scene, {lo, lo, lo}, {hi, lo, lo}, {hi, hi, lo}, {lo, hi, lo}, {0, 0, 1},  bsdf, emission, radiance);
    add_wall(scene, {lo, lo, hi}, {hi, lo, hi}, {hi, hi, hi}, {lo, hi, hi}, {0, 0, -1}, bsdf, emission, radiance);
    add_wall(scene, {lo, lo, lo}, {lo, hi, lo}, {lo, hi, hi}, {lo, lo, hi}, {1, 0, 0},  bsdf, emission, radiance);
    add_wall(scene, {hi, lo, lo}, {hi, hi, lo}, {hi, hi, hi}, {hi, lo, hi}, {-1, 0, 0}, bsdf, emission, radiance);
    add_wall(scene, {lo, lo, lo}, {hi, lo, lo}, {hi, lo, hi}, {lo, lo, hi}, {0, 1, 0},  bsdf, emission, radiance);
    add_wall(scene, {lo, hi, lo}, {hi, hi, lo}, {hi, hi, hi}, {lo, hi, hi}, {0, -1, 0}, bsdf, emission, radiance);
}

// ── The furnace: a uniform environment, and one object in it ─────────────

inline Scene uniform_environment(const Bsdf& object, double radius = 1.0,
                                 double half = 10.0) {
    Scene scene;
    add_box(scene, half, Bsdf{GreyLambert{Flat{0.0}}}, Flat{1.0}, 1.0);
    scene.add(Surface{Sphere{Vec3{0, 0, 0}, radius}, object});
    scene.finalise();
    return scene;
}

// The Lambertian case, which is what the furnace has always meant by it.
inline Scene uniform_environment(double object_albedo, double radius = 1.0,
                                 double half = 10.0) {
    return uniform_environment(Bsdf{GreyLambert{Flat{object_albedo}}}, radius, half);
}

// ── The cavity: L = Le / (1 - rho), and nothing in it ────────────────────

inline Scene emitting_cavity(double rho, double emitted = 1.0, double half = 10.0) {
    Scene scene;
    add_box(scene, half, Bsdf{GreyLambert{Flat{rho}}}, Flat{1.0}, emitted);
    scene.finalise();
    return scene;
}

// The closed form, written where the scene is, so that the two cannot drift
// apart without somebody editing the same twelve lines.
constexpr double cavity_radiance(double rho, double emitted = 1.0) {
    return emitted / (1.0 - rho);
}

} // namespace enclosure

} // namespace app
