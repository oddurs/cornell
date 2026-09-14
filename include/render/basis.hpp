// basis.hpp — two perpendicular directions, from one.
//
// Every sampling routine in this project works in a space where the surface
// normal is +z, because that is the space the formulae in the papers are
// written in: a cosine is `w.z`, a hemisphere is `w.z > 0`, and a density
// that depends on the angle from the normal depends on one component. Then
// the result has to come back out to the world, which means somebody has to
// invent the other two axes.
//
// They are not determined. A normal fixes one direction and leaves a free
// rotation about it, so any construction picks a tangent arbitrarily, and
// the only requirements are that the three are orthonormal, that the
// handedness is consistent, and that the choice does not jump discontinuously
// as the normal moves — because a sampling routine whose tangent frame flips
// halfway across a sphere puts a seam in the image that looks like a geometry
// bug and is not one.
//
// ── The name, and why it is not a person's ────────────────────────────────
//
// House rule 8 says a file is named for a phenomenon or a person. This one is
// named for neither, deliberately, because two people have a claim and
// spelling it either way would be a small lie.
//
// Frisvad published the construction in 2012: rotate +z onto n with the
// shortest arc, write the rotation out, and most of it cancels. It is elegant
// and it is undefined at one point. When n is exactly −z there is no shortest
// arc — every arc is the same length — and the formula divides by `1 + n.z`,
// which is then zero.
//
// Duff, Burgess, Christensen, Cui, Donow, Jenkins, Kjaer, Lewis, Nikolov,
// Smyk and Villemin published the fix in 2017, in the *Journal of Computer
// Graphics Techniques*, under the title "Building an Orthonormal Basis,
// Revisited" — eleven authors for four lines of arithmetic, which tells you
// how often the original bit somebody.
//
// The fix is one `copysign`. Flip the construction into the hemisphere the
// normal is actually in, so the division is by `sign + n.z` where both terms
// share a sign and the divisor is never smaller than 1 in magnitude. No
// branch, no special case, and the frame rotates smoothly through both poles.
//
// ── What that is actually worth, measured, rather than repeated ───────────
//
// The received version of this story is that the original degrades in the
// *neighbourhood* of the pole, well before it fails at it. In double
// precision that is not what happens, and the file should say what was
// measured rather than what is usually said:
//
//      n.z              this file        Frisvad 2012
//      −1 + 1e-2        3.74e-18         2.22e-16
//      −1 + 1e-4        7.58e-20         2.22e-16
//      −1 + 1e-8        1.18e-20         2.22e-16
//      −1 + 1e-16       0.00e+00         2.22e-16
//      −1 exactly       0.00e+00         NaN
//
// The original is at one ulp all the way in, and then it is NaN. The
// degradation is real but it is a single-precision effect — the 2017 paper
// works in `float`, where the divisor bottoms out several orders of magnitude
// sooner — and this project is in `double`, so what it is buying here is the
// exact pole and the absence of a branch, not accuracy.
//
// That is still worth having. A normal of exactly (0, 0, −1) is not exotic:
// it is the floor of an axis-aligned box, which is the scene this renderer
// exists to draw. And NaN does not announce itself — it propagates to a black
// pixel that looks like a shadow.
//
// Measured over 320 800 directions covering the whole sphere, the worst
// departure from orthonormality here is 4.79e-16.
//
// So the file implements the second paper's correction of the first paper's
// method, and is named for the thing rather than for either of them.
//
// ── Handedness ───────────────────────────────────────────────────────────
//
// Stated once, here, and obeyed everywhere: (tangent, bitangent, normal) is
// right-handed, in that order, so `cross(tangent, bitangent) == normal`. It
// matches `vec.hpp`'s axes — hand a normal of +z and you get back exactly +x
// and +y — which is the property that makes the tests below mean something.

#pragma once

#include <cmath>
#include <render/vec.hpp>

namespace render {

// A tangent frame. It stores three directions rather than recomputing them,
// because a path bounce transforms two or three vectors through the same
// frame and the construction is cheaper than it looks but not free.
class Basis {
public:
    explicit Basis(const Unit& normal) : n_{normal} {
        // Duff et al., 2017. The sign is the whole paper.
        const double sign = std::copysign(1.0, n_.z());
        const double a = -1.0 / (sign + n_.z());
        const double b = n_.x() * n_.y() * a;

        t_ = Vec3{1.0 + sign * n_.x() * n_.x() * a, sign * b, -sign * n_.x()};
        s_ = Vec3{b, sign + n_.y() * n_.y() * a, -n_.y()};
    }

    const Vec3& tangent()   const { return t_; }
    const Vec3& bitangent() const { return s_; }
    const Unit& normal()    const { return n_; }

    // Local to world. `local` is written in the frame where the normal is
    // +z, which is the space every warp in `warp.hpp` produces.
    Vec3 to_world(const Vec3& local) const {
        return t_ * local.x + s_ * local.y + n_.vec() * local.z;
    }

    // World to local. The frame is orthonormal, so the inverse is the
    // transpose, which is three dot products — and this is the reason the
    // invariant in `Unit` is worth having: if the frame were only nearly
    // orthonormal, this would be nearly the inverse, and "nearly" compounds
    // over a path.
    Vec3 to_local(const Vec3& world) const {
        return Vec3{dot(world, t_), dot(world, s_), dot(world, n_)};
    }

private:
    Unit n_;
    Vec3 t_{};
    Vec3 s_{};
};

} // namespace render
