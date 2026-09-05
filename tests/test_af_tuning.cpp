// Copyright (c) 2026 Daniel Boles. MIT — see LICENSE.
//
// Numbers here are derived from the sourced research behind
// docs/superpowers/specs/2026-09-05-defwhistler-pitch-spec.md.
// They are not round because the physics is not round. Do not "tidy" them.

#include "../src/dsp/AfTuning.hpp"
#include "../src/dsp/AfTables.hpp"
#include "../src/dsp/AfDrift.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
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

void testTables()
{
    using namespace af::tuning;

    // Every shipped table must say where it came from. Two of them are
    // contested reconstructions and the UI has to be able to admit that.
    for (auto id : {TableId::Equal12, TableId::HarmonicSeries,
                    TableId::ChathamNoWave, TableId::Shruti5Limit,
                    TableId::ShrutiEqual22}) {
        const Table& t = table(id);
        require(t.name != nullptr && t.name[0] != '\0', "table has a name");
        require(t.provenance != nullptr && t.provenance[0] != '\0',
                "table has a provenance");
        require(t.count > 0, "table is not empty");
    }

    // 12-TET.
    const Table& eq = table(TableId::Equal12);
    require(eq.count == 12, "12-TET has 12 degrees");
    near(eq.cents[0], 0.f, 1e-4f, "12-TET root");
    near(eq.cents[7], 700.f, 1e-4f, "12-TET fifth is exactly 700c");

    // Chatham: D A D A (7/4) D. The flat C is the whole point.
    const Table& ch = table(TableId::ChathamNoWave);
    require(ch.count == 6, "Chatham tuning has six strings");
    near(ch.cents[1], 701.955f, 1e-2f, "Chatham A is a just fifth");
    near(ch.cents[4], 2400.f + 968.826f, 1e-2f, "Chatham flat C is 7/4");

    // The 5-limit shruti reconstruction must sum to exactly an octave. If it
    // does not, the ratio list has a typo, and this catches it.
    const Table& sh = table(TableId::Shruti5Limit);
    require(sh.count == 22, "22 shrutis");
    near(sh.cents[0], 0.f, 1e-3f, "shruti 1 is the root");
    near(sh.cents[21], 1109.775f, 1e-2f, "shruti 22 is 243/128");
    for (int i = 1; i < sh.count; ++i)
        require(sh.cents[i] > sh.cents[i - 1], "shrutis ascend");

    // The equal-shruti reading: 22-EDO.
    const Table& se = table(TableId::ShrutiEqual22);
    require(se.count == 22, "22 equal shrutis");
    near(se.cents[1], 54.5455f, 1e-3f, "22-EDO step");

    // 7/4 has no place in the shruti set — nearest members are 27c away.
    // This verifies data integrity; a collision would indicate a data-entry error.
    float nearest = 1e9f;
    for (int i = 0; i < sh.count; ++i)
        nearest = std::fmin(nearest, std::fabs(sh.cents[i] - 968.826f));
    require(nearest > 25.f, "7/4 is not in the shruti set");
}

void testDrift()
{
    using namespace af::tuning;

    // Coherence 0: every voice shares one drift signal. Ratios are preserved
    // exactly and the whole stack transposes — a tape machine, not a chorus.
    {
        Drift d;
        d.reset(8, 1234u);
        d.setRate(1.f); d.setDepth(20.f); d.setCoherence(0.f);
        for (int i = 0; i < 500; ++i) d.process(1.f / 48000.f);
        const float v0 = d.centsFor(0);
        for (int v = 1; v < 8; ++v)
            near(d.centsFor(v), v0, 1e-4f, "coherent drift moves every voice alike");
    }

    // Coherence 1: independent per voice. This is the mechanism behind twenty
    // violins, a hundred guitars, and a Moog that will not stay in tune.
    {
        Drift d;
        d.reset(8, 1234u);
        d.setRate(1.f); d.setDepth(20.f); d.setCoherence(1.f);
        for (int i = 0; i < 500; ++i) d.process(1.f / 48000.f);
        bool differs = false;
        for (int v = 1; v < 8; ++v)
            differs = differs || std::fabs(d.centsFor(v) - d.centsFor(0)) > 0.5f;
        require(differs, "incoherent drift separates the voices");
    }

    // Depth is a bound, not a suggestion: an unbounded excursion detunes a
    // drone into a different note.
    {
        Drift d;
        d.reset(4, 99u);
        d.setRate(3.f); d.setDepth(10.f); d.setCoherence(1.f);
        for (int i = 0; i < 200000; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "drift stays finite");
                require(std::fabs(d.centsFor(v)) <= 10.f + 1e-3f,
                        "drift stays within depth");
            }
        }
    }

    // Zero depth must be exactly silent, so a preset can turn it off.
    {
        Drift d;
        d.reset(4, 7u);
        d.setRate(2.f); d.setDepth(0.f); d.setCoherence(1.f);
        for (int i = 0; i < 1000; ++i) d.process(1.f / 48000.f);
        for (int v = 0; v < 4; ++v)
            near(d.centsFor(v), 0.f, 1e-6f, "zero depth is no drift");
    }

    // Determinism with reset: one instance, reset twice with same seed, must
    // reproduce. This catches if reset() forgets to reinitialise sharedPhase_.
    {
        Drift a;
        a.reset(4, 42u);
        a.setRate(2.f); a.setDepth(15.f); a.setCoherence(1.f);
        for (int i = 0; i < 1000; ++i) a.process(1.f/48000.f);
        float first_run[4] = {a.centsFor(0), a.centsFor(1), a.centsFor(2), a.centsFor(3)};

        a.reset(4, 42u);
        a.setRate(2.f); a.setDepth(15.f); a.setCoherence(1.f);
        for (int i = 0; i < 1000; ++i) a.process(1.f/48000.f);
        for (int v = 0; v < 4; ++v)
            near(a.centsFor(v), first_run[v], 1e-6f,
                 "drift recalls after reset with same seed");
    }

    // NaN/Infinity rejection: setters must not accept non-finite input, and
    // process() must never produce NaN or Infinity in output.
    {
        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float inf = std::numeric_limits<float>::infinity();

        // setRate(NaN) must be rejected.
        Drift d;
        d.reset(4, 99u);
        d.setRate(1.f);  // set a known-good value first
        d.setRate(nan);  // try to corrupt it
        d.setDepth(10.f); d.setCoherence(1.f);
        for (int i = 0; i < 500; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "rate=NaN rejected, output stays finite");
            }
        }

        // setRate(Infinity) must be rejected.
        d.reset(4, 99u);
        d.setRate(inf);
        d.setDepth(10.f); d.setCoherence(1.f);
        for (int i = 0; i < 500; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "+Inf rate rejected, output stays finite");
            }
        }

        // setDepth(NaN) must be rejected.
        d.reset(4, 99u);
        d.setRate(1.f);
        d.setDepth(nan);
        d.setCoherence(1.f);
        for (int i = 0; i < 500; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "depth=NaN rejected, output stays finite");
            }
        }

        // setDepth(Infinity) must be rejected.
        d.reset(4, 99u);
        d.setRate(1.f);
        d.setDepth(inf);
        d.setCoherence(1.f);
        for (int i = 0; i < 500; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "+Inf depth rejected, output stays finite");
            }
        }

        // setCoherence(NaN) must be rejected.
        d.reset(4, 99u);
        d.setRate(1.f); d.setDepth(10.f);
        d.setCoherence(nan);
        for (int i = 0; i < 500; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "coherence=NaN rejected, output stays finite");
            }
        }

        // setCoherence(Infinity) must be rejected.
        d.reset(4, 99u);
        d.setRate(1.f); d.setDepth(10.f);
        d.setCoherence(inf);
        for (int i = 0; i < 500; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "+Inf coherence rejected, output stays finite");
            }
        }

        // Negative values are rejected (setRate, setDepth).
        d.reset(4, 99u);
        d.setRate(-5.f);
        d.setDepth(-10.f);
        d.setCoherence(1.f);
        for (int i = 0; i < 500; ++i) {
            d.process(1.f / 48000.f);
            for (int v = 0; v < 4; ++v) {
                require(std::isfinite(d.centsFor(v)), "negative rate/depth rejected, output stays finite");
            }
        }
    }
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
    testTables();
    testDrift();

    if (failures > 0) {
        std::cerr << failures << " AfTuning check(s) failed\n";
        return 1;
    }
    std::cout << "AfTuning tests passed\n";
    return 0;
}
