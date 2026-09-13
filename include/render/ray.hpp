// ray.hpp — an origin, a direction, and the interval that is the point.
//
// A ray in geometry is a half-line: it starts somewhere, it goes in some
// direction, and it goes on forever. A ray in a renderer is never that. It is
// always a *segment*, and the two numbers that bound it carry more of the
// program's meaning than the six that describe where it points.
//
// This is the whole argument for the type existing at all, rather than
// passing an origin and a direction as two loose vectors:
//
//      t_max is how a shadow ray asks a different question. "Is there
//      anything between this surface and that light" is not "what is the
//      nearest thing along this direction" — it is cheaper, it can stop at
//      the first hit rather than sorting for the closest, and the only thing
//      that distinguishes it from its expensive cousin is that somebody set
//      an upper bound. Lose the bound and the query becomes the wrong query.
//
//      t_min is where a lie gets told, and this file is where it is admitted.
//
// ── The lie in t_min ──────────────────────────────────────────────────────
//
// A path bounces. The renderer intersects a surface, computes the hit point
// as `origin + t * direction`, and casts a new ray from it. That point is not
// on the surface. It cannot be: `origin + t * direction` is three multiplies
// and three adds in floating point, each rounded, and the result lands a few
// ulps to one side or the other of the triangle it is supposed to be on. Land
// on the wrong side and the new ray immediately intersects the surface it
// just left, at some t near zero. The bounce goes nowhere, the path terminates
// against its own starting point, and the image fills with black speckle that
// looks exactly like noise and does not go away with more samples.
//
// Every renderer's first fix is `t_min = 1e-4`. It works, on the scene it was
// tuned on. It is a length, it is being compared against a scene whose scale
// the author happened to know, and it fails in both directions: too small for
// a hit a kilometre away, where the ulps are metres wide, and too large for a
// scene in millimetres, where it punches visible holes in contact shadows.
// The constant is not a tolerance. It is the author's scene, written down.
//
// The right fix does not scale an epsilon; it offsets the ray origin along
// the normal by an amount computed from the magnitude of the coordinates
// themselves, in integer arithmetic on the float bit patterns — Wächter and
// Binder, 2019. It is exact, it has no tuning parameter, and it needs a
// surface normal at the hit, which this file does not have because there is
// nothing to hit yet.
//
// v0.1 shipped the epsilon and said so. It is gone: `waechter.hpp` offsets
// the ray's *origin* by a number of ulps rather than its parameter by a
// length, `t_min` below defaults to zero, and there is no constant with units
// left anywhere in this part of the project.
//
// What remains here is the statement of the problem, because the file that
// solves it is about floating point and this one is about rays.
//
// ── What is not modelled ──────────────────────────────────────────────────
//
// Ray differentials. A ray that carries the derivative of its own position
// and direction with respect to the pixel it came from tells a texture lookup
// how large a footprint it covers, which is how a renderer picks a mip level
// instead of aliasing. They are genuinely useful and they are not free: every
// bounce has to differentiate its own scattering. There are no textures in
// this project until v0.4 at the earliest, and there may never be any. Noted
// here so that adding them is a decision rather than a surprise.
//
// Time. A ray with a timestamp is how motion blur works, and nothing in this
// project moves. If something ever does, it is a field here and a shutter in
// `camera.hpp`, and both of those files have to change together.

#pragma once

#include <limits>
#include <render/vec.hpp>

namespace render {

struct Ray {
    Vec3 origin{};
    Unit direction{};

    // The segment. Everything outside [t_min, t_max] is not this ray's
    // business, and an intersection routine that ignores the bounds is not
    // faster — it is answering a question nobody asked.
    // Zero, not an epsilon. A ray that starts on a surface is prevented from
    // hitting it by where it starts, which is `waechter.hpp`'s job, and not
    // by refusing to look at the first fraction of a millimetre of it.
    double t_min = 0.0;
    double t_max = std::numeric_limits<double>::infinity();

    constexpr Vec3 at(double t) const { return origin + direction.vec() * t; }

    // Whether a parameter found by an intersection routine is actually on
    // this ray. Written here, once, rather than as a pair of comparisons
    // repeated in every primitive, where one of them will eventually be
    // spelled `<` instead of `<=` and the difference will be a rendering
    // artefact somebody spends an afternoon on.
    constexpr bool holds(double t) const { return t >= t_min && t <= t_max; }
};

// There was a `shadow_ray` here, and it is gone rather than ported.
//
// It built its segment out of two subtractions of `ray_epsilon`, one at each
// end, which is the construction this milestone exists to delete. It was also
// never called: at v0.2 a light is found only by paths that happen to wander
// into it, so nothing in the project asks whether a particular point can see
// a particular light.
//
// The question arrives properly in v0.8 with direct light sampling, and the
// routine that answers it will need offsetting at *both* ends — away from the
// surface it leaves and away from the light it lands on — which is a
// different function from the one that was here. Writing it now, uncalled and
// untested against a real occlusion, would be guessing at v0.8's interface
// and leaving a plausible-looking thing for somebody to trust.

} // namespace render
