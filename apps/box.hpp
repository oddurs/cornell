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
// ── Why the walls are still grey ─────────────────────────────────────────
//
// The box is famous for a red wall and a green wall, and both walls here are
// the same neutral grey.
//
// In v0.2 that was because a material could not be asked what it reflected at
// a particular wavelength. It can now — `bsdf.hpp` gained the argument, and
// the lamp below emits a tabulated D65 through exactly that route — so the
// obstacle is gone and the walls are grey for the other reason.
//
// They are grey because the red one is *measured*. Item 0053 is the spectral
// reflectance Cornell published for that wall, and it arrives in v0.4 with
// the rest of the box. Typing a plausible red here, now that it would finally
// work, is more tempting than it was and exactly as wrong: it is the
// difference between a renderer that reproduces a photograph and one that
// looks about right.
//
// The route for an RGB somebody types *does* exist as of v0.3 — item 0044's
// upsampling — and its own criterion says the Cornell walls may not go
// through it.

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

// The lamp's brightness, in spectral radiance where its spectrum is 1.
// Chosen so the walls land in a range the preview can show. Item 0054 is the
// light's measured emission spectrum and geometry.
//
// Its *spectrum* is no longer a placeholder: it is D65, tabulated, the same
// illuminant `srgb.hpp` builds its matrix around — so a white wall in this
// room renders as white on a calibrated display for a reason rather than by
// arrangement. A real fixture is not D65 either, which is what 0054 fixes.
inline constexpr double lamp_radiance = 0.12;

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
                     const render::Emission& emission,
                     double radiance) {
    using namespace render;
    Triangle first{a, b, c};
    Triangle second{a, c, d};
    if (dot(first.normal(), should_face) < 0.0) {
        first = Triangle{a, c, b};
        second = Triangle{a, d, c};
    }
    scene.add(Surface{first, bsdf, emission, radiance});
    scene.add(Surface{second, bsdf, emission, radiance});
}

} // namespace detail

// The `scale` argument exists for one reason: item 0038 claims that
// `waechter.hpp`'s offset is scale-invariant, and the way to check that is to
// build the same room a thousand times larger and a thousand times smaller
// and see whether the picture changes. Radiance is invariant under a uniform
// scaling of a scene — every length in the transport cancels — so the two
// images must agree bit for bit, and if they do not, the offset has a length
// hidden in it somewhere.
inline render::Scene box(double scale = 1.0,
                         const render::cie::Illuminant& lamp = render::cie::d65) {
    using namespace render;

    Scene scene;
    const Bsdf wall = GreyLambert{Flat{wall_albedo}};

    const double h = box_size * scale / 2.0;   // half width
    const double t = box_size * scale;         // ceiling height
    const double back = -box_size * scale;

    // floor, ceiling, back, left, right — each facing into the room.
    detail::add_quad(scene, {-h, 0, 0}, {h, 0, 0}, {h, 0, back}, {-h, 0, back},
                     {0, 1, 0}, wall, render::Flat{0.0}, 0.0);
    detail::add_quad(scene, {-h, t, 0}, {h, t, 0}, {h, t, back}, {-h, t, back},
                     {0, -1, 0}, wall, render::Flat{0.0}, 0.0);
    detail::add_quad(scene, {-h, 0, back}, {h, 0, back}, {h, t, back}, {-h, t, back},
                     {0, 0, 1}, wall, render::Flat{0.0}, 0.0);
    detail::add_quad(scene, {-h, 0, 0}, {-h, 0, back}, {-h, t, back}, {-h, t, 0},
                     {1, 0, 0}, wall, render::Flat{0.0}, 0.0);
    detail::add_quad(scene, {h, 0, 0}, {h, 0, back}, {h, t, back}, {h, t, 0},
                     {-1, 0, 0}, wall, render::Flat{0.0}, 0.0);

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
                     Bsdf{GreyLambert{Flat{0.0}}},   // a lamp that also
                                                     // reflects is a lamp
                                                     // with a shade, and
                                                     // this one has none
                     lamp,
                     lamp_radiance);
    return scene;
}

} // namespace app
