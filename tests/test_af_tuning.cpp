// Copyright (c) 2026 Daniel Boles. MIT — see LICENSE.
//
// Numbers here are derived from the sourced research behind
// docs/superpowers/specs/2026-09-05-defwhistler-pitch-spec.md.
// They are not round because the physics is not round. Do not "tidy" them.

#include "../src/dsp/AfTuning.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
int failures = 0;

void near(float actual, float expected, float tol, const std::string& what)
{
    if (std::fabs(actual - expected) > tol) {
        std::cerr << "FAIL: " << what << " expected " << expected
                  << " got " << actual << '\n';
        ++failures;
    }
}

void require(bool cond, const std::string& what)
{
    if (!cond) { std::cerr << "FAIL: " << what << '\n'; ++failures; }
}

void testPartials()
{
    using namespace af::tuning;
    // The harmonic series expression the codebase already uses, in cents.
    near(partialCents(1), 0.f, 1e-4f, "partial 1 is the root");
    near(partialCents(2), 1200.f, 1e-3f, "partial 2 is an octave");
    near(partialCents(3), 1901.955f, 1e-2f, "partial 3");
    // Reduced to the octave, partial 3 is the just fifth.
    near(partialCents(3) - 1200.f, 701.955f, 1e-2f, "just fifth is 701.955c");
    // The 7th partial: 31.17 cents flat of 12-TET. This is the no-wave interval.
    near(partialCents(7) - 2400.f, 968.826f, 1e-2f, "7/4 is 968.826c");
}

void testSpacingCollapse()
{
    using namespace af::tuning;
    // Above roughly partial 30 the series stops being pitches and becomes a
    // beating field. These three numbers are the design boundary.
    near(partialSpacingCents(32), 53.27f, 0.01f, "spacing at n=32");
    near(partialSpacingCents(64), 26.84f, 0.01f, "spacing at n=64");
    near(partialSpacingCents(127), 13.58f, 0.01f, "spacing at n=127");
    require(partialSpacingCents(2) > partialSpacingCents(64),
            "spacing shrinks as n rises");
}

void testBeatRate()
{
    using namespace af::tuning;
    // beat_hz = f * cents / 1731.234. The whole reason a cents-denominated
    // detune changes character across the keyboard.
    near(beatHz(110.f, 10.f), 0.635f, 0.005f, "10c at 110Hz");
    near(beatHz(880.f, 10.f), 5.083f, 0.005f, "10c at 880Hz");
    near(beatHz(440.f, 31.174f), 7.923f, 0.01f, "7/4 against 12-TET at 440Hz");
    // Linear in both arguments.
    near(beatHz(440.f, 20.f), 2.f * beatHz(440.f, 10.f), 1e-3f,
         "beat rate is linear in cents");
    near(beatHz(0.f, 10.f), 0.f, 1e-6f, "no frequency, no beating");
}

void testRoundTrip()
{
    using namespace af::tuning;
    for (float c : {-1200.f, -31.17f, 0.f, 1.f, 701.955f, 1200.f, 3986.f})
        near(ratioToCents(centsToRatio(c)), c, 1e-2f, "cents round-trip");
    near(centsToRatio(0.f), 1.f, 1e-6f, "0 cents is unity");
    near(centsToRatio(1200.f), 2.f, 1e-4f, "1200 cents is 2:1");
}

void testVinylWow()
{
    using namespace af::tuning;
    // Depth is radius-dependent: a loop from the run-out wobbles more than one
    // from the lead-in. That is the whole character of vinyl wow.
    near(vinylWowCents(0.5f, 100.f), 8.656f, 0.01f, "0.5mm at 100mm");
    near(vinylWowCents(0.5f, 50.f), 17.312f, 0.02f, "same error, inner groove");
    near(vinylWowCents(0.f, 100.f), 0.f, 1e-6f, "a centred pressing does not wow");
    // Once per revolution.
    near(vinylWowRateHz(33.333333f), 0.5556f, 0.001f, "33 1/3 rpm");
    near(vinylWowRateHz(45.f), 0.75f, 0.001f, "45 rpm");
    near(vinylWowRateHz(78.f), 1.30f, 0.005f, "78 rpm");
}

void testGuards()
{
    using namespace af::tuning;
    // A zero or negative radius is a caller bug; return 0 rather than infinity,
    // because an inf reaching a phase accumulator produces NaN audio.
    require(std::isfinite(vinylWowCents(0.5f, 0.f)), "zero radius stays finite");
    require(std::isfinite(vinylWowRateHz(0.f)), "zero rpm stays finite");
    require(std::isfinite(partialCents(0)), "partial 0 stays finite");
}
} // namespace

int main()
{
    testPartials();
    testSpacingCollapse();
    testBeatRate();
    testRoundTrip();
    testVinylWow();
    testGuards();

    if (failures > 0) {
        std::cerr << failures << " AfTuning check(s) failed\n";
        return 1;
    }
    std::cout << "AfTuning tests passed\n";
    return 0;
}
