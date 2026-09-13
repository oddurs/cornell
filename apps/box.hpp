// box.hpp — a box, provisionally.
//
// This is not the Cornell box and the distinction matters enough to put at
// the top of the file. The Cornell box is a measured object: plywood, built
// and painted and photographed at the Program of Computer Graphics, published
// with the spectral reflectance of every wall and the emission spectrum and
// geometry of its light. Rendering *that* is v0.4, in `cornell.hpp`, and it
// is the whole reason this project chose this subject.
//
// What this is: five walls and a lamp, with round numbers, built so that the
// integrator has something with corners to be checked in. It lives in `apps/`
// rather than in `include/render/` because it is a test rig rather than a
// scene, and putting it next to the optics would be claiming a status it has
// not got.
//
// Three things about it are placeholders and are marked as such below: the
// dimensions, the albedo, and the light's emission. Every one of them is a
// number somebody typed, which is the thing this project exists not to do,
// and every one of them is replaced in v0.4 by a number somebody measured.
//
// ── Why the walls are grey ───────────────────────────────────────────────
//
// The box is famous for a red wall and a green wall, and both walls here are
// the same neutral grey. That is not laziness; it is the spectral discipline
// arriving before the colour does.
//
// A red wall is a *spectrum*: a reflectance that is high at long wavelengths
// and low at short ones. `Reflectance` can already hold that — it is four
// numbers at four wavelengths — but the four wavelengths differ from path to
// path, so a wall's reflectance has to be evaluated *at the wavelengths this
// path is carrying*, and nothing in `bsdf.hpp` passes them yet. Adding a
// wavelength argument is a change to the contract every material signs, and
// it belongs with `cie.hpp` in v0.3, where there is finally an observer to
// turn the answer into something a monitor can show.
//
// So: grey walls, and the red one arrives with its measured spectrum rather
// than with a number chosen to look right. Typing `(0.8, 0.1, 0.1)` here to
// get a red wall two milestones early would be exactly the thing the README
// promises this project never does.

#pragma once

#include <render/lambert.hpp>
#include <render/scene.hpp>
#include <render/spectrum.hpp>
#include <render/triangle.hpp>
#include <render/vec.hpp>

namespace app {

// ── The placeholders, together, so they are easy to find and delete ──────

// Two metres on a side. The real box is not two metres on a side, and item
// 0051 is "find out exactly what Cornell published, and what units it is in".
inline constexpr double box_size = 2.0;

// A flat 0.7 at every wavelength. Real paint is not flat and is not 0.7;
// item 0053 is the measured spectral reflectances.
inline constexpr double wall_albedo = 0.7;

// Spectral radiance, W·m⁻²·sr⁻¹·m⁻¹, chosen so that the walls land in a
// range the preview can show. Item 0054 is the light's measured emission
// spectrum and geometry.
inline constexpr double lamp_radiance = 12.0;

// ── The geometry ────────────────────────────────────────────────────────
//
// x from −1 to +1, y from 0 at the floor to 2 at the ceiling, z from −2 at
// the back wall to 0 at the opening. The front is missing, because the
// camera looks in through it — which is how the real box was photographed,
// and is the reason a Cornell box image always has that framing.

namespace detail {

// Two triangles, wound so that the normal comes out on the side the caller
// asks for. Getting this wrong makes a wall that light passes through or a
// lamp that emits into the ceiling, and both look like physics.
inline void add_quad(render::Scene& scene,
                     const render::Vec3& a, const render::Vec3& b,
                     const render::Vec3& c, const render::Vec3& d,
                     const render::Vec3& should_face,
                     const render::Bsdf& bsdf,
                     const render::Radiance& emission) {
    using namespace render;
    Triangle first{a, b, c};
    Triangle second{a, c, d};
    if (dot(first.normal(), should_face) < 0.0) {
        first = Triangle{a, c, b};
        second = Triangle{a, d, c};
    }
    scene.add(Surface{first, bsdf, emission});
    scene.add(Surface{second, bsdf, emission});
}

} // namespace detail

// The `scale` argument exists for one reason: item 0038 claims that
// `waechter.hpp`'s offset is scale-invariant, and the way to check that is to
// build the same room a thousand times larger and a thousand times smaller
// and see whether the picture changes. Radiance is invariant under a uniform
// scaling of a scene — every length in the transport cancels — so the two
// images must agree bit for bit, and if they do not, the offset has a length
// hidden in it somewhere.
inline render::Scene box(double scale = 1.0) {
    using namespace render;

    Scene scene;
    const Bsdf wall = Lambert{Reflectance{wall_albedo}};
    const Radiance dark{};

    const double h = box_size * scale / 2.0;   // half width
    const double t = box_size * scale;         // ceiling height
    const double back = -box_size * scale;

    // floor, ceiling, back, left, right — each facing into the room.
    detail::add_quad(scene, {-h, 0, 0}, {h, 0, 0}, {h, 0, back}, {-h, 0, back},
                     {0, 1, 0}, wall, dark);
    detail::add_quad(scene, {-h, t, 0}, {h, t, 0}, {h, t, back}, {-h, t, back},
                     {0, -1, 0}, wall, dark);
    detail::add_quad(scene, {-h, 0, back}, {h, 0, back}, {h, t, back}, {-h, t, back},
                     {0, 0, 1}, wall, dark);
    detail::add_quad(scene, {-h, 0, 0}, {-h, 0, back}, {-h, t, back}, {-h, t, 0},
                     {1, 0, 0}, wall, dark);
    detail::add_quad(scene, {h, 0, 0}, {h, 0, back}, {h, t, back}, {h, t, 0},
                     {-1, 0, 0}, wall, dark);

    // The lamp: a panel just below the ceiling, facing down. Just below
    // rather than in it, because two coplanar surfaces are a coin toss for
    // every ray that reaches them, and a lamp that is sometimes behind the
    // ceiling flickers in a way that looks like a sampling bug.
    const double lamp_half = 0.3 * scale;
    const double lamp_y = t - 0.01 * scale;
    const double lamp_z = back / 2.0;
    detail::add_quad(scene,
                     {-lamp_half, lamp_y, lamp_z - lamp_half},
                     { lamp_half, lamp_y, lamp_z - lamp_half},
                     { lamp_half, lamp_y, lamp_z + lamp_half},
                     {-lamp_half, lamp_y, lamp_z + lamp_half},
                     {0, -1, 0},
                     Bsdf{Lambert{Reflectance{0.0}}},   // a lamp that also
                                                        // reflects is a lamp
                                                        // with a shade, and
                                                        // this one has none
                     Radiance{lamp_radiance});
    return scene;
}

} // namespace app
