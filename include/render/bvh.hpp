// bvh.hpp — the one file here that is not about light.
//
// Every other file in this project earns its place by explaining something.
// This one earns its place by making the others runnable, and it should be
// read with that difference in mind: there is no physics below, nothing is
// derived from a measurement, and if the renderer were fast enough without it
// the right thing would be to delete it.
//
// It is an index. The scene is a list of triangles; a ray needs the nearest
// one it hits; testing all of them is correct and costs the whole list. A
// bounding volume hierarchy wraps groups of triangles in boxes, wraps groups
// of boxes in boxes, and lets a ray skip a subtree whose box it misses. The
// answer is identical either way — item 0058 is the check that it is — and
// only the time changes.
//
// ── The surface area heuristic, which is the only derived thing here ─────
//
// Where to split a set of triangles is a choice, and the obvious choices are
// bad. Splitting at the median gives balanced trees and ignores that a
// balanced tree over badly-shaped boxes is slower than a lopsided one over
// tight boxes. Splitting at the spatial midpoint ignores where the geometry
// actually is.
//
// The surface area heuristic asks the right question: what does this split
// *cost*, in expected ray-triangle tests? A ray that hits the parent box
// enters a child with probability equal to the ratio of their surface areas —
// which is a fact about convex bodies and uniformly distributed lines, not a
// heuristic, and is the one piece of real derivation in this file. So:
//
//      cost(split) = area(left)  * count(left)
//                  + area(right) * count(right)
//
// dropped of the constant factors that do not affect which split wins. Try
// every candidate, take the cheapest, and stop when no split beats leaving
// the node as a leaf.
//
// "Every candidate" is too many — there are as many as there are triangles,
// per axis. So the centroids are dropped into sixteen bins per axis and the
// fifteen splits between bins are evaluated by two sweeps, one from each end,
// accumulating bounds and counts. That turns an O(n²) search into O(n), and
// sixteen is enough that the chosen plane is almost always the one a full
// search would have found.
//
// ── Thirty-two bytes, and what it costs to get there ─────────────────────
//
// A flat array, not a pointer tree. The left child of node `i` is always
// `i + 1` — it is built depth-first, so it is the next node written — and only
// the right child needs an index. That removes a pointer, removes the
// allocation per node, and puts siblings near each other in memory.
//
// Six floats of bounds, an offset, and one word carrying both the primitive
// count and the split axis: 32 bytes, half a cache line, two nodes per line.
//
// The count and the axis share a word because they have to. They were a
// `uint16` each, which is ample axis and is *not* ample count: a leaf holding
// 65 536 primitives wrapped to zero, zero means "inner node", and the
// traversal then read the offset as a child index. Measured before it was
// fixed — 70 008 coincident primitives built a tree reaching 4 472 of them
// and losing 65 536, silently, with a correct-looking image, which is the
// same failure mode as the `grow` bug below. Thirty bits of count and two of
// axis costs nothing and cannot do that.
//
// `float` bounds in a `double` renderer is the one place this project
// deliberately loses precision, and it is safe only because of what a bound is
// for. A bounding box that is slightly too big costs a few wasted intersection
// tests. One that is slightly too small silently drops geometry. So the
// conversion rounds *outward* — `nextafter` towards infinity on the maxima and
// towards minus infinity on the minima — and the float box is guaranteed to
// contain the double box it came from. That is the whole of the argument for
// halving the node size, and it is checked by item 0058 rather than asserted.
//
// ── Measured ─────────────────────────────────────────────────────────────
//
// Against the same scene intersected exhaustively, which `scene.hpp` keeps
// for exactly this purpose. Random rays, random origins inside the box:
//
//      primitives   nodes    brute (s)   bvh (s)   speedup
//              38      25       0.242      0.152     1.59x
//             238     163       0.663      0.257     2.58x
//           2 038   1 375       4.745      0.448    10.60x
//          20 038  13 485       9.612      0.163    59.00x
//
// Build: 190 ns per primitive at two thousand, 222 ns at twenty thousand —
// 4.5 ms for the larger, against a render that takes seconds. The build is
// not worth optimising and is written for clarity.
//
// The Cornell box is the first row. 1.59x is a modest return, and it is the
// honest one: thirty-eight large triangles arranged as a hollow box is close
// to the worst case for a hierarchy, because every subset of the walls has
// nearly the bounding box of all of them.
//
// ── Against a median split, which is the comparison that found a bug ─────
//
// The item that asked for this file said a median split is "two to four times
// slower" and that the comment should carry the measured figure rather than
// the claim. The measured figure is smaller:
//
//      primitives   surface area heuristic   median split
//              38                    1.59x          1.62x
//             238                    2.58x          2.15x
//           2 038                   10.60x          8.56x
//          20 038                   59.00x         49.89x
//
// About twenty per cent at scale, and nothing at all on a small scene. The
// heuristic is still right — it wins wherever it matters and never loses by
// much — but two-to-four times overstates it.
//
// That comparison earned its place a different way. The first time it was
// run, the median split beat the heuristic by a factor of seventeen, which is
// not a tuning difference: it is a bug report. `Bounds::grow(const Bounds&)`
// was growing by empty boxes, whose `low` is +infinity and whose `high` is
// -infinity, so a single empty bin poisoned the running bounds for every
// candidate after it, every split costed infinity, none was ever chosen, and
// nodes became leaves of arbitrary size. The tree had 757 nodes for twenty
// thousand primitives instead of 13 485.
//
// It produced correct images the whole time. It was simply not accelerating
// anything, and nothing would have revealed that except measuring it against
// the thing it was supposed to beat.

// ── What is not modelled ─────────────────────────────────────────────────
//
// SIMD packet traversal, ray reordering, spatial splits, treelet rotation,
// wide branching factors, and the rest. Each is worth two to five times and
// costs the reader everything. This project trades throughput for legibility,
// and this is the file where that trade is most expensive and most deliberate:
// a production BVH is perhaps five times faster than this one and perhaps ten
// times longer.
//
// Refitting, so a moving scene can reuse the tree. Nothing moves.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include <render/ray.hpp>
#include <render/vec.hpp>

namespace render {

// An axis-aligned box, in doubles, used during the build.
struct Bounds {
    Vec3 low{ std::numeric_limits<double>::infinity(),
              std::numeric_limits<double>::infinity(),
              std::numeric_limits<double>::infinity()};
    Vec3 high{-std::numeric_limits<double>::infinity(),
              -std::numeric_limits<double>::infinity(),
              -std::numeric_limits<double>::infinity()};

    void grow(const Vec3& p) {
        low.x = std::fmin(low.x, p.x);   high.x = std::fmax(high.x, p.x);
        low.y = std::fmin(low.y, p.y);   high.y = std::fmax(high.y, p.y);
        low.z = std::fmin(low.z, p.z);   high.z = std::fmax(high.z, p.z);
    }

    // Growing by an empty box must do nothing, and the guard is the whole
    // reason this is not a one-liner. An empty box has `low` at +infinity and
    // `high` at -infinity, so growing by its corners sets the result's low to
    // -infinity and its high to +infinity — an infinite box, reported as
    // infinite area, which then loses every comparison it takes part in.
    //
    // Without this the binned sweep below silently broke whenever a bin came
    // out empty: the running bounds were poisoned from that bin onward, every
    // candidate split after it costed infinity, no split was ever chosen, and
    // the node became a leaf of whatever size it happened to be. It cost a
    // factor of seventeen and looked like a design flaw in the heuristic.
    void grow(const Bounds& b) {
        if (b.empty()) return;
        grow(b.low);
        grow(b.high);
    }

    Vec3 extent() const { return high - low; }
    Vec3 centre() const { return (low + high) * 0.5; }
    bool empty() const { return high.x < low.x; }

    // Twice the surface area, which is what the heuristic compares. The
    // factor of two is common to every candidate and cancels.
    double half_area() const {
        if (empty()) return 0.0;
        const Vec3 d = extent();
        return d.x * d.y + d.y * d.z + d.z * d.x;
    }
};

// Thirty-two bytes. See the header.
struct BvhNode {
    float low[3]{};
    float high[3]{};
    std::uint32_t offset = 0;      // leaf: first primitive; inner: right child
    std::uint32_t packed = 0;      // count in the top 30 bits, axis in the low 2

    static constexpr std::uint32_t max_count = (1u << 30) - 1;

    constexpr std::uint32_t count() const { return packed >> 2; }
    constexpr std::uint32_t axis()  const { return packed & 3u; }
    constexpr bool leaf()           const { return count() > 0; }

    constexpr void set(std::uint32_t primitives, std::uint32_t split_axis) {
        packed = (primitives << 2) | (split_axis & 3u);
    }
};

static_assert(sizeof(BvhNode) == 32, "a node must be half a cache line");

// The packing round-trips at sizes a `uint16` could not hold, which is the
// whole point of it. Checked by the compiler because the failure it replaces
// needed seventy thousand primitives to show up at runtime.
static_assert([] {
    BvhNode n;
    n.set(70000, 2);
    return n.count() == 70000 && n.axis() == 2 && n.leaf();
}(), "a leaf must be able to hold more primitives than a uint16");

static_assert([] {
    BvhNode n;
    n.set(BvhNode::max_count, 3);
    return n.count() == BvhNode::max_count && n.axis() == 3;
}(), "the packing must reach its own stated maximum");

static_assert([] {
    BvhNode n;
    n.set(0, 1);
    return !n.leaf() && n.axis() == 1;
}(), "a count of zero must still mean an inner node");

namespace detail {

// Round outward, so the float box contains the double box. A box that is
// slightly too big wastes work; one that is slightly too small loses
// geometry.
inline float outward_low(double v) {
    const float f = float(v);
    return double(f) <= v ? f : std::nextafter(f, -std::numeric_limits<float>::infinity());
}

inline float outward_high(double v) {
    const float f = float(v);
    return double(f) >= v ? f : std::nextafter(f, std::numeric_limits<float>::infinity());
}

} // namespace detail

class Bvh {
public:
    static constexpr int bins = 16;
    static constexpr std::size_t max_leaf = 4;

    // The traversal stack is a fixed array, so the build must not produce a
    // tree deeper than it can hold.
    //
    // This is not hypothetical. When the centroids are exponentially spaced
    // every split peels one primitive off, so 998 of them at x = 2^i built a
    // tree of depth 107 against a stack of 64 — a buffer overflow on the
    // first traversal, on every worker thread at once, caught by
    // AddressSanitizer and by nothing else. The Cornell box is depth 5, which
    // is exactly why it survived.
    static constexpr int max_depth = 60;
    static constexpr int stack_size = 64;
    static_assert(stack_size > max_depth, "the stack must hold the deepest path");

    // Build over a list of bounds, one per primitive. The caller keeps the
    // primitives; this returns the order to visit them in.
    void build(const std::vector<Bounds>& item_bounds) {
        const std::size_t n = item_bounds.size();
        order_.resize(n);
        for (std::size_t i = 0; i < n; ++i) order_[i] = std::uint32_t(i);

        nodes_.clear();
        if (n == 0) return;
        nodes_.reserve(2 * n);

        centroids_.resize(n);
        for (std::size_t i = 0; i < n; ++i) centroids_[i] = item_bounds[i].centre();

        split(item_bounds, 0, n, 1);
    }

    const std::vector<BvhNode>& nodes() const { return nodes_; }
    const std::vector<std::uint32_t>& order() const { return order_; }

    // Walk the tree, handing each candidate primitive to `test`, which
    // returns true if it found a closer hit and has shortened the ray.
    template <class Test>
    void traverse(const Ray& ray, Ray& shortened, Test&& test) const {
        if (nodes_.empty()) return;

        const Vec3 inverse{1.0 / ray.direction.x(), 1.0 / ray.direction.y(),
                           1.0 / ray.direction.z()};

        std::uint32_t stack[stack_size];
        int depth = 0;
        stack[depth++] = 0;

        while (depth > 0) {
            const BvhNode& node = nodes_[stack[--depth]];
            if (!hits_box(node, ray.origin, inverse, shortened.t_max)) continue;

            if (node.leaf()) {
                for (std::uint32_t i = 0; i < node.count(); ++i)
                    test(order_[node.offset + i]);
                continue;
            }

            // Near child first. The ray's direction along the split axis says
            // which that is, and visiting it first means the far one is often
            // rejected by a t_max that has already shrunk.
            const std::uint32_t a = std::uint32_t(&node - nodes_.data()) + 1;
            const std::uint32_t b = node.offset;
            const bool flip = component(ray.direction, int(node.axis())) < 0.0;
            stack[depth++] = flip ? a : b;
            stack[depth++] = flip ? b : a;
        }
    }

private:
    static bool hits_box(const BvhNode& node, const Vec3& origin,
                         const Vec3& inverse, double t_max) {
        double near = 0.0, far = t_max;
        for (int k = 0; k < 3; ++k) {
            const double o = component(origin, k);
            const double d = component(inverse, k);
            double t0 = (double(node.low[std::size_t(k)]) - o) * d;
            double t1 = (double(node.high[std::size_t(k)]) - o) * d;
            if (t0 > t1) std::swap(t0, t1);
            near = t0 > near ? t0 : near;
            far  = t1 < far  ? t1 : far;
            if (far < near) return false;
        }
        return true;
    }

    void store(BvhNode& node, const Bounds& b) const {
        node.low[0]  = detail::outward_low(b.low.x);
        node.low[1]  = detail::outward_low(b.low.y);
        node.low[2]  = detail::outward_low(b.low.z);
        node.high[0] = detail::outward_high(b.high.x);
        node.high[1] = detail::outward_high(b.high.y);
        node.high[2] = detail::outward_high(b.high.z);
    }

    std::uint32_t split(const std::vector<Bounds>& item_bounds,
                        std::size_t first, std::size_t count, int depth) {
        const std::uint32_t index = std::uint32_t(nodes_.size());
        nodes_.push_back(BvhNode{});

        Bounds node_bounds, centroid_bounds;
        for (std::size_t i = first; i < first + count; ++i) {
            node_bounds.grow(item_bounds[order_[i]]);
            centroid_bounds.grow(centroids_[order_[i]]);
        }
        store(nodes_[index], node_bounds);

        // A leaf, and the split taken when nothing better is available.
        //
        // The heuristic can decline for reasons that have nothing to do with
        // the leaf being small: coincident centroids, a zero-width spread, or
        // no candidate beating the parent. Making a leaf of whatever is left
        // is then unbounded, which is how 65 536 primitives went missing. So
        // past a size worth splitting, an arbitrary split by index is taken —
        // a worse tree than the heuristic would build, and an enormously
        // better one than a leaf of seventy thousand.
        const auto make_leaf = [&] {
            nodes_[index].offset = std::uint32_t(first);
            nodes_[index].set(std::uint32_t(count), 0);
            return index;
        };

        const auto split_arbitrarily = [&] {
            const std::size_t half = count / 2;
            nodes_[index].set(0, 0);
            split(item_bounds, first, half, depth + 1);
            nodes_[index].offset = split(item_bounds, first + half, count - half, depth + 1);
            return index;
        };

        // Stop here, unless stopping would overflow the count.
        const auto stop = [&] {
            return count > BvhNode::max_count ? split_arbitrarily() : make_leaf();
        };

        if (count <= max_leaf || depth >= max_depth) return stop();

        const Vec3 spread = centroid_bounds.extent();
        const int axis = spread.x > spread.y ? (spread.x > spread.z ? 0 : 2)
                                             : (spread.y > spread.z ? 1 : 2);
        const double low = component(centroid_bounds.low, axis);
        const double width = component(spread, axis);
        if (width <= 0.0) return count > max_leaf ? split_arbitrarily() : stop();

        // Bin the centroids.
        Bounds bin_bounds[bins];
        std::size_t bin_count[bins] = {};
        const double scale = double(bins) / width;
        for (std::size_t i = first; i < first + count; ++i) {
            const std::uint32_t item = order_[i];
            int b = int((component(centroids_[item], axis) - low) * scale);
            b = b < 0 ? 0 : (b >= bins ? bins - 1 : b);
            bin_bounds[std::size_t(b)].grow(item_bounds[item]);
            ++bin_count[std::size_t(b)];
        }

        // Two sweeps: bounds and counts accumulated from each end, so every
        // candidate split is O(1) to evaluate.
        double left_area[bins - 1];
        std::size_t left_count[bins - 1];
        Bounds running;
        std::size_t running_count = 0;
        for (int i = 0; i < bins - 1; ++i) {
            running.grow(bin_bounds[i]);
            running_count += bin_count[i];
            left_area[i] = running.half_area();
            left_count[i] = running_count;
        }

        double best_cost = std::numeric_limits<double>::infinity();
        int best_split = -1;
        Bounds right;
        std::size_t right_count = 0;
        for (int i = bins - 2; i >= 0; --i) {
            right.grow(bin_bounds[i + 1]);
            right_count += bin_count[i + 1];
            if (left_count[i] == 0 || right_count == 0) continue;

            const double cost = left_area[i] * double(left_count[i])
                              + right.half_area() * double(right_count);
            if (cost < best_cost) { best_cost = cost; best_split = i; }
        }

        // Leaving it whole costs one traversal of every primitive.
        const double leaf_cost = node_bounds.half_area() * double(count);
        if (best_split < 0 || best_cost >= leaf_cost)
            return count > max_leaf ? split_arbitrarily() : stop();

        const auto middle = std::partition(
            order_.begin() + std::ptrdiff_t(first),
            order_.begin() + std::ptrdiff_t(first + count),
            [&](std::uint32_t item) {
                int b = int((component(centroids_[item], axis) - low) * scale);
                b = b < 0 ? 0 : (b >= bins ? bins - 1 : b);
                return b <= best_split;
            });

        const std::size_t left_size =
            std::size_t(middle - (order_.begin() + std::ptrdiff_t(first)));
        if (left_size == 0 || left_size == count) return split_arbitrarily();

        nodes_[index].set(0, std::uint32_t(axis));
        split(item_bounds, first, left_size, depth + 1);        // left is index + 1
        nodes_[index].offset =
            split(item_bounds, first + left_size, count - left_size, depth + 1);
        return index;
    }

    std::vector<BvhNode> nodes_;
    std::vector<std::uint32_t> order_;
    std::vector<Vec3> centroids_;
};

} // namespace render
