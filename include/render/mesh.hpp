// mesh.hpp — the thing that holds triangles, and the reason it is barely a
// thing at all.
//
// A mesh is an indexed set of triangles: positions stored once, faces naming
// three of them, and optionally a normal per vertex. The indexing is what
// makes a million-triangle model fit in memory — a shared vertex is stored
// once rather than six times — and it is also, for this project, almost
// pointless.
//
// The Cornell box is 32 triangles. Indexing saves a few hundred bytes and
// costs an indirection on every intersection, and if that were the whole
// story this file would not exist and `cornell.hpp` would emit triangles
// directly.
//
// What it is actually for is the part that is not about memory: a mesh is
// where a *surface* is described rather than a pile of unrelated faces. Two
// triangles that share vertices share them exactly, so the edge between them
// is the same two points to the last bit for both — which is the difference
// between a seam that is watertight by construction and one that is
// watertight because the numbers happened to be typed identically twice.
// Item 0055 measured what a shared edge does when the arithmetic disagrees;
// this is how it is made not to.
//
// ── Flat by default ──────────────────────────────────────────────────────
//
// A mesh with no vertex normals produces flat triangles, and every surface in
// the Cornell box is flat. `with_normals` exists so that the distinction
// `triangle.hpp` draws between geometric and shading normals has something to
// be a distinction about, and so that the first curved thing to arrive does
// not have to invent the machinery.
//
// ── What is not modelled ─────────────────────────────────────────────────
//
// Loading a mesh from a file. There is no OBJ reader, no glTF, no PLY. The
// only geometry this project has is published in a table and is compiled in,
// and a parser would be a dependency-shaped hole in house rule 4 — not
// because it needs a library, but because it needs a *format*, and a format
// is somebody else's specification that this repository would then have to
// track.
//
// Per-face materials. A mesh is one surface with one BSDF, and the box is
// assembled from several meshes rather than one mesh with a material index.
// That is the right shape for six walls and the wrong shape for a model with
// a thousand parts.

#pragma once

#include <cstddef>
#include <vector>

#include <render/triangle.hpp>
#include <render/vec.hpp>

namespace render {

class Mesh {
public:
    // Positions, and faces naming three of them each. Counter-clockwise seen
    // from the front, which `triangle.hpp` defines as the side the geometric
    // normal points out of.
    struct Face {
        std::size_t a = 0;
        std::size_t b = 0;
        std::size_t c = 0;
    };

    Mesh(std::vector<Vec3> positions, std::vector<Face> faces)
        : positions_{std::move(positions)}, faces_{std::move(faces)} {}

    // The same mesh with a normal per vertex, which makes its triangles
    // smooth. `normals[i]` belongs to `positions[i]`.
    Mesh(std::vector<Vec3> positions, std::vector<Face> faces, std::vector<Unit> normals)
        : positions_{std::move(positions)}, faces_{std::move(faces)},
          normals_{std::move(normals)} {}

    std::size_t face_count() const { return faces_.size(); }
    bool smooth() const { return !normals_.empty(); }

    // One face, as a self-contained triangle.
    //
    // A copy, deliberately. The alternative is a triangle holding a pointer
    // back into this mesh, which is smaller and turns every intersection into
    // a pointer chase and every mesh into a lifetime problem. Ninety-six
    // bytes copied once at build time is the cheaper mistake.
    Triangle triangle(std::size_t face) const {
        const Face& f = faces_[face];
        Triangle t{positions_[f.a], positions_[f.b], positions_[f.c]};
        if (smooth()) {
            t.na = normals_[f.a];
            t.nb = normals_[f.b];
            t.nc = normals_[f.c];
        }
        return t;
    }

    std::vector<Triangle> triangles() const {
        std::vector<Triangle> out;
        out.reserve(faces_.size());
        for (std::size_t i = 0; i < faces_.size(); ++i) out.push_back(triangle(i));
        return out;
    }

    // A quadrilateral, as the two triangles the published Cornell geometry
    // means by one. Every surface in that data set is a quad, and splitting
    // them consistently — both triangles sharing the a–c diagonal — is what
    // makes the shared edge exact rather than nearly exact.
    static std::vector<Face> quad(std::size_t a, std::size_t b, std::size_t c, std::size_t d) {
        return {Face{a, b, c}, Face{a, c, d}};
    }

private:
    std::vector<Vec3> positions_;
    std::vector<Face> faces_;
    std::vector<Unit> normals_;
};

} // namespace render
