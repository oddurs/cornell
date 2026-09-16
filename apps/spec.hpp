// spec.hpp — the box as a specification, with everything that follows from it.
//
// `cornell.hpp` holds what Cornell published. This prints it, and then prints
// the quantities that are *consequences* of it — areas, a solid angle, an
// irradiance — computed from the vertices rather than quoted.
//
// The point is that a reader can check them. Every number below is either a
// figure from the source, which can be compared against the page, or an
// arithmetic consequence of those figures, which can be worked out by hand on
// the back of an envelope. Nothing in between.
//
// Where a quantity can be obtained two ways, it is, and both are shown. The
// lamp's solid angle has a closed form and can also be integrated
// numerically; printing both and their difference is the only kind of
// self-check a specification can offer.

#pragma once

#include <cmath>
#include <cstdio>

#include <render/cornell.hpp>
#include <render/si.hpp>

namespace app {

inline int spec() {
    using namespace render;
    using namespace render::cornell;

    std::printf("The Cornell box, as published.\n");
    std::printf("  Cornell University Program of Computer Graphics, \"Cornell Box Data\",\n");
    std::printf("  last updated 2 February 2005. The page is gone from the live web;\n");
    std::printf("  see item 0051 and the archive link in cornell.hpp.\n\n");

    std::printf("MEASURED — geometry, in metres (published in millimetres)\n");
    std::printf("  height                     %.4f\n", mm::height / 1e3);
    std::printf("  depth                      %.4f\n", mm::depth / 1e3);
    std::printf("  floor width, far edge      %.4f\n", mm::floor_far / 1e3);
    std::printf("  floor width, near edge     %.4f     <- not the same\n", mm::floor_near / 1e3);
    std::printf("  ceiling width              %.4f     <- nor this\n", mm::ceiling_x / 1e3);
    std::printf("  the box was measured, not drawn; the walls are not perpendicular\n\n");

    const Vec3 f0 = at(mm::floor_far, 0, 0), f1 = at(0, 0, 0);
    const Vec3 f2 = at(0, 0, mm::depth),     f3 = at(mm::floor_near, 0, mm::depth);
    const double floor_area = quad_area(f0, f1, f2, f3);

    const Vec3 b0 = at(mm::floor_near, 0, mm::depth), b1 = at(0, 0, mm::depth);
    const Vec3 b2 = at(0, mm::height, mm::depth), b3 = at(mm::ceiling_x, mm::height, mm::depth);
    const double back_area = quad_area(b0, b1, b2, b3);

    const Vec3 r0 = at(0, 0, mm::depth), r1 = at(0, 0, 0);
    const Vec3 r2 = at(0, mm::height, 0), r3 = at(0, mm::height, mm::depth);
    const double right_area = quad_area(r0, r1, r2, r3);

    const Vec3 l0 = at(mm::floor_far, 0, 0), l1 = at(mm::floor_near, 0, mm::depth);
    const Vec3 l2 = at(mm::ceiling_x, mm::height, mm::depth), l3 = at(mm::ceiling_x, mm::height, 0);
    const double left_area = quad_area(l0, l1, l2, l3);

    std::printf("DERIVED — areas, in square metres, as two triangles each\n");
    std::printf("  floor                      %.6f\n", floor_area);
    std::printf("  back wall                  %.6f\n", back_area);
    std::printf("  right wall (green)         %.6f\n", right_area);
    std::printf("  left wall (red)            %.6f\n", left_area);
    std::printf("  lamp                       %.6f\n", light_area);
    std::printf("  the two side walls differ by %.2e m2, which is the measurement\n\n",
                std::fabs(left_area - right_area));

    // The lamp, from the point on the floor directly beneath its centre —
    // which is not quite the centre of the floor, and the closed form below
    // is only exact on the axis, so the two have to be the same point.
    const double lamp_cx = 0.5 * (mm::light_x0 + mm::light_x1) / 1e3;
    const double lamp_cz = 0.5 * (mm::light_z0 + mm::light_z1) / 1e3;
    const double half_x = 0.5 * (mm::light_x1 - mm::light_x0) / 1e3;
    const double half_z = 0.5 * (mm::light_z1 - mm::light_z0) / 1e3;
    const double drop = mm::height / 1e3;

    const double closed_form = axial_solid_angle(half_x, half_z, drop);

    // The same thing by brute force, from a point directly under the lamp.
    // Two routes to one number; if they disagree, one of them is wrong.
    const Vec3 under{lamp_cx, 0.0, lamp_cz};
    double numeric = 0.0, projected = 0.0;
    const int N = 2000;
    const double cell = (2.0 * half_x / N) * (2.0 * half_z / N);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            const double x = lamp_cx - half_x + (double(i) + 0.5) * (2.0 * half_x / N);
            const double z = lamp_cz - half_z + (double(j) + 0.5) * (2.0 * half_z / N);
            const Vec3 to{x - under.x, drop, z - under.z};
            const double d2 = length_squared(to);
            const double cosine = drop / std::sqrt(d2);     // both normals are vertical
            numeric   += cell * cosine / d2;
            projected += cell * cosine * cosine / d2;
        }
    }

    std::printf("DERIVED — the lamp, seen from the floor directly beneath it\n");
    std::printf("  solid angle, closed form   %.6f sr\n", closed_form);
    std::printf("  solid angle, integrated    %.6f sr\n", numeric);
    std::printf("  they differ by             %.2e sr\n", std::fabs(closed_form - numeric));
    std::printf("  projected solid angle      %.6f sr\n\n", projected);

    std::printf("CALIBRATED — the lamp's level, because Cornell's scale is arbitrary\n");
    std::printf("  radiance factor            %.4f\n", light_radiance);
    std::printf("  flux, pi L A               %.6f W per unit of the published spectrum\n\n",
                light_flux_for(light_radiance));

    std::printf("DERIVED — first bounce at the centre of the floor\n");
    std::printf("  irradiance is the projected solid angle times the radiance, and the\n");
    std::printf("  floor then returns that times its reflectance over pi.\n");
    const double at_555 = emission.at(555.0e-9) * light_radiance;
    std::printf("  lamp radiance at 555 nm    %.4f\n", at_555);
    std::printf("  irradiance at 555 nm       %.4f\n", projected * at_555);
    std::printf("  floor reflectance at 555   %.4f\n", white.at(555.0e-9));
    std::printf("  floor radiance at 555 nm   %.4f\n",
                projected * at_555 * white.at(555.0e-9) * si::inv_pi);
    std::printf("  (direct light only; the measured image includes every later bounce)\n");
    return 0;
}

} // namespace app
