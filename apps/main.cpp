// main.cpp — the dispatcher, and the list of witnesses.
//
// There is no physics in this file and there never will be. It reads `argv`,
// it decides which instrument was asked for, and it calls it. That is the
// whole job, and the reason it gets a file to itself is the second half of
// the job, which is more useful than the first:
//
//      `./cornell` with no arguments prints what the instruments are, what
//      each one witnesses, and which milestone it arrives in.
//
// It is the fastest way for somebody who has just cloned this to find out
// what has been built and what has only been argued for. A README can say
// anything; this list is printed by the program, so it can only say what the
// program can do — except for the `built` column, which is a hand-kept `bool`
// and is therefore the one thing in the table that can lie. It is checkable by
// running the instrument, which is more than a README offers.
//
// The table below is a copy of the roadmap, and house rule 6 says copies rot.
// It is kept because a list of seven lines is worth the maintenance and
// because `cairn check` will not catch it drifting; when an instrument moves
// milestone, it moves here too, in the same commit.

#include <cstdio>
#include <cstdlib>
#include <string_view>

#include "chi2.hpp"
#include "converge.hpp"
#include "furnace.hpp"
#include "render.hpp"
#include "replay.hpp"
#include "spec.hpp"
#include "verify.hpp"
#include "spectrum.hpp"

namespace {

struct Witness {
    std::string_view name;
    std::string_view milestone;
    std::string_view witnesses;
    bool built;
};

// In the order they arrive, which is the order a reader should meet them.
// `replay` is not in this table, and that is deliberate rather than an
// oversight. The table is the list of instruments — things that witness a
// claim about the physics — and replay witnesses nothing. It is a debugger's
// hand, reachable from the address an assertion prints, and it belongs in the
// usage lines underneath rather than in the list a reader is meant to work
// through in order.
constexpr Witness witnesses[] = {
    {"render",   "v0.1", "the image itself",                                        true },
    {"spec",     "v0.4", "the box as a specification, everything derived",        true },
    {"spectrum", "v0.3", "any spectrum in the project, with its chromaticity",      true },
    {"furnace",  "v0.5", "energy conservation, as a pass/fail you can see",         true },
    {"chi2",     "v0.5", "that sample() and pdf() describe the same distribution",  true },
    {"converge", "v0.5", "that RMSE falls as N^-1/2, or the estimator is biased",   true },
    {"verify",   "v0.4", "every physical claim the project makes, in one run",      true },
    {"swatch",   "v0.6", "the metals, rendered from nothing but citations",         false},
};

void print_witnesses() {
    std::printf("cornell — a box in a lab, modelled from first principles, for no reason.\n\n");
    for (const Witness& w : witnesses)
        std::printf("  %-9.*s %-5.*s %-6s %.*s\n",
                    int(w.name.size()),      w.name.data(),
                    int(w.milestone.size()), w.milestone.data(),
                    w.built ? "built" : "",
                    int(w.witnesses.size()), w.witnesses.data());
    std::printf("\n  ./cornell render [width] [--spp N]\n");
    std::printf("  ./cornell spectrum [d65|e|x|y|z|red-wall|green-wall|white-wall]\n");
    std::printf("  ./cornell spec\n");
    std::printf("  ./cornell verify\n"
                "  ./cornell furnace [--bsdf lambert] [--rho R] [--no-image]\n"
                "  ./cornell chi2\n"
                "  ./cornell converge\n"
                "  ./cornell replay x,y,sample [width]   one path, again, on one thread\n");
    std::printf("  --tonemap clip|reinhard   a choice, not physics; see tonemap.hpp\n");
    std::printf("  --lamp d65|a              daylight, or tungsten\n");
    std::printf("  --no-adapt                do not chromatically adapt; see bradford.hpp\n");
    std::printf("  --threads N               default is what the machine reports\n");
    std::printf("  The height is derived from the width and the shape of the film.\n");
}

// Digits only, and no error reporting beyond refusing to change the default.
// An argument parser that accepts "50%%" and silently renders at the default
// size is worse than one that refuses, but this is v0.1 and the instruments
// that take real arguments arrive with real parsing in v0.5.
int integer_or(std::string_view text, int fallback) {
    // Accumulated in a `long long` and refused past INT_MAX. In an `int` this
    // was signed overflow — undefined behaviour, which UBSan traps and which
    // otherwise wraps to whatever it wraps to, so `./cornell render
    // 99999999999` was accepted as a plausible-looking positive width.
    long long value = 0;
    for (const char c : text) {
        if (c < '0' || c > '9') return fallback;
        value = value * 10 + (c - '0');
        if (value > 2147483647LL) return fallback;
    }
    return text.empty() || value <= 0 ? fallback : int(value);
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_witnesses();
        return 0;
    }

    const std::string_view command{argv[1]};

    if (command == "spec") return app::spec();

    if (command == "verify") return app::verify();

    if (command == "chi2") return app::chi2();

    if (command == "converge") return app::converge();

    if (command == "replay") {
        if (argc < 3) {
            std::fprintf(stderr, "cornell: replay wants an address, as x,y,sample.\n");
            return 1;
        }
        app::RenderSettings settings;
        for (int i = 3; i < argc; ++i)
            settings.width = integer_or(std::string_view{argv[i]}, settings.width);
        return app::replay(std::string_view{argv[2]}, settings);
    }

    if (command == "furnace") {
        std::string_view model{"lambert"};
        double rho = 1.0;
        bool image = true;
        for (int i = 2; i < argc; ++i) {
            const std::string_view arg{argv[i]};
            if (arg == "--bsdf" && i + 1 < argc) model = argv[++i];
            else if (arg == "--rho" && i + 1 < argc) rho = std::atof(argv[++i]);
            else if (arg == "--no-image") image = false;
        }
        return app::furnace(model, rho, image);
    }

    if (command == "spectrum") {
        return app::spectrum(argc > 2 ? std::string_view{argv[2]} : std::string_view{"d65"});
    }

    if (command == "render") {
        app::RenderSettings settings;
        // `--spp N`, and a bare number is still the width. The sample count
        // no longer has to be a perfect square: v0.1's lattice needed a side,
        // and the sampler in v0.2 does not.
        for (int i = 2; i < argc; ++i) {
            const std::string_view arg{argv[i]};
            if (arg == "--spp" && i + 1 < argc) settings.spp = integer_or(argv[++i], settings.spp);
            else if (arg == "--tonemap" && i + 1 < argc) settings.curve = app::tone_curve_named(argv[++i]);
            else if (arg == "--lamp" && i + 1 < argc) settings.tungsten = std::string_view{argv[++i]} == "a";
            else if (arg == "--no-adapt") settings.adapt = false;
            else if (arg == "--threads" && i + 1 < argc) settings.threads = integer_or(argv[++i], 0);
            else settings.width = integer_or(arg, settings.width);
        }
        return app::render(settings);
    }

    for (const Witness& w : witnesses) {
        if (w.name == command) {
            std::fprintf(stderr, "cornell: %.*s is not built yet. It arrives in %.*s.\n",
                         int(w.name.size()), w.name.data(),
                         int(w.milestone.size()), w.milestone.data());
            return 1;
        }
    }

    std::fprintf(stderr, "cornell: no instrument called '%.*s'.\n\n",
                 int(command.size()), command.data());
    print_witnesses();
    return 1;
}
