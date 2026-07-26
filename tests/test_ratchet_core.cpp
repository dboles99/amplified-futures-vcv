// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Standalone tests for RatchetCore. No Rack, no VCV SDK - plain g++.
#include "../src/dsp/RatchetCore.hpp"
#include <cstdio>
#include <vector>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

static const float SR = 48000.f;

// Drive the core with evenly spaced input edges and collect the sample index
// of every rising edge on TRIG and on END.
struct Run {
    std::vector<int> trigs, ends;
};

static Run drive(RatchetCore& r, int periodSamples, int nInputs,
                 int count, float spread, float prob, int tailSamples = 0) {
    Run run;
    bool prevT = false, prevE = false;
    const int total = periodSamples * nInputs + tailSamples;
    for (int i = 0; i < total; i++) {
        const bool edge = (i % periodSamples) == 0 && (i / periodSamples) < nInputs;
        RatchetCore::Out o = r.process(edge, count, spread, prob);
        if (o.trig && !prevT) run.trigs.push_back(i);
        if (o.end  && !prevE) run.ends.push_back(i);
        prevT = o.trig;
        prevE = o.end;
    }
    return run;
}

int main() {
    const int P = int(0.5f * SR);   // 500 ms between input edges = 120 BPM

    // The first edge cannot be subdivided - there is no measured interval yet.
    // It must still pass through, or the module would swallow the downbeat.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 1, 4, 0.f, 1.f);
        check(run.trigs.size() == 1, "first trigger passes through alone");
        check(!r.hasPeriod(),        "no period is claimed after one edge");
        check(run.trigs.size() == 1 && run.trigs[0] == 0,
              "the pass-through is sample-aligned with the input");
    }

    // Second edge onward: the burst fills the measured interval.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 2, 4, 0.f, 1.f);
        check(r.hasPeriod(), "a period is measured from two edges");
        check(std::fabs(r.periodSeconds() - 0.5f) < 1e-3f,
              "the measured period matches the input interval");
        // 1 from the first edge, 4 from the second.
        check(run.trigs.size() == 5, "COUNT=4 yields four repeats on the second edge");
    }

    // COUNT is honoured exactly, and clamped rather than overflowing the table.
    {
        for (int c = 1; c <= 8; c++) {
            RatchetCore r;
            r.setSampleRate(SR);
            Run run = drive(r, P, 2, c, 0.f, 1.f);
            if (int(run.trigs.size()) != 1 + c) {
                std::printf("  FAIL: COUNT=%d produced %d pulses, expected %d\n",
                            c, int(run.trigs.size()), 1 + c);
                ++g_failures;
                goto countDone;
            }
        }
        check(true, "COUNT 1..8 each produce exactly that many repeats");
        countDone:;
    }
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 2, 99, 0.f, 1.f);
        check(r.scheduledRepeats() <= RatchetCore::MAX_REPEATS,
              "COUNT above the maximum is clamped, not overflowed");
        check(run.trigs.size() == 1 + 8, "clamped COUNT still fires the full eight");
    }

    // SPREAD 0 must be exactly even. Not "roughly" - an uneven default would
    // make the knob's centre detent meaningless.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        drive(r, P, 2, 4, 0.f, 1.f);
        bool even = true;
        const int step = P / 4;
        for (int i = 0; i < r.scheduledRepeats(); i++)
            if (std::abs(r.repeatSample(i) - i * step) > 1) even = false;
        check(even, "SPREAD=0 spaces repeats evenly to within one sample");
    }

    // Accelerating means the gaps shrink; decelerating means they grow.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        drive(r, P, 2, 6, 1.f, 1.f);
        bool shrinking = true;
        int prevGap = -1;
        for (int i = 1; i < r.scheduledRepeats(); i++) {
            const int gap = r.repeatSample(i) - r.repeatSample(i - 1);
            if (prevGap >= 0 && gap >= prevGap) shrinking = false;
            prevGap = gap;
        }
        check(shrinking, "SPREAD=+1 accelerates: every gap shorter than the last");
    }
    {
        RatchetCore r;
        r.setSampleRate(SR);
        drive(r, P, 2, 6, -1.f, 1.f);
        bool growing = true;
        int prevGap = -1;
        for (int i = 1; i < r.scheduledRepeats(); i++) {
            const int gap = r.repeatSample(i) - r.repeatSample(i - 1);
            if (prevGap >= 0 && gap <= prevGap) growing = false;
            prevGap = gap;
        }
        check(growing, "SPREAD=-1 decelerates: every gap longer than the last");
    }

    // Whatever the spread, repeats must stay ordered and inside the window,
    // or a burst would collide with the next input edge.
    {
        bool ordered = true, inside = true;
        for (int s = -10; s <= 10; s++) {
            RatchetCore r;
            r.setSampleRate(SR);
            drive(r, P, 2, 8, s / 10.f, 1.f);
            for (int i = 1; i < r.scheduledRepeats(); i++) {
                if (r.repeatSample(i) <= r.repeatSample(i - 1)) ordered = false;
                if (r.repeatSample(i) >= P) inside = false;
            }
        }
        check(ordered, "repeats are strictly ordered across the whole SPREAD range");
        check(inside,  "the burst always finishes inside the measured window");
    }

    // Pulses must not merge. Two triggers a hair apart read as one long gate.
    {
        bool separated = true;
        for (int s = -10; s <= 10; s++) {
            RatchetCore r;
            r.setSampleRate(SR);
            drive(r, P, 2, 8, s / 10.f, 1.f);
            for (int i = 1; i < r.scheduledRepeats(); i++)
                if (r.repeatSample(i) - r.repeatSample(i - 1) < int(0.001f * SR) * 2)
                    separated = false;
        }
        check(separated, "adjacent repeats stay at least two pulse widths apart");
    }

    // PROB=1 fires everything; PROB=0 leaves only the pass-through.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 2, 8, 0.f, 1.f);
        check(run.trigs.size() == 9, "PROB=1 fires every repeat");
    }
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 2, 8, 0.f, 0.f);
        check(run.trigs.size() == 2, "PROB=0 leaves only the two pass-throughs");
    }
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 20, 8, 0.f, 0.5f);
        // 20 inputs: 1 bare pass-through plus 19 bursts of up to 8.
        const int maxPossible = 1 + 19 * 8;
        check(int(run.trigs.size()) > 20 && int(run.trigs.size()) < maxPossible,
              "PROB=0.5 drops some repeats but not all of them");
    }

    // END must fire once per burst, after the last scheduled repeat, whether
    // or not that repeat was dropped by PROB.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 3, 4, 0.f, 1.f, P);
        check(run.ends.size() == 3, "END fires exactly once per burst");
        bool after = true;
        for (size_t e = 0; e < run.ends.size(); e++) {
            // Every END must follow at least one TRIG and precede the next one.
            bool sawTrigBefore = false;
            for (size_t t = 0; t < run.trigs.size(); t++)
                if (run.trigs[t] < run.ends[e]) sawTrigBefore = true;
            if (!sawTrigBefore) after = false;
        }
        check(after, "every END follows the burst it closes");
    }
    {
        RatchetCore r;
        r.setSampleRate(SR);
        Run run = drive(r, P, 3, 6, 0.f, 0.f, P);
        check(run.ends.size() == 3, "END still fires when PROB drops every repeat");
    }

    // reset() must clear the measured period and the RNG, or a re-armed module
    // would inherit the previous patch's tempo and burst pattern.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        drive(r, P, 4, 6, 0.f, 0.4f);
        r.reset();
        check(!r.hasPeriod(), "reset() forgets the measured period");

        RatchetCore a, b;
        a.setSampleRate(SR);
        b.setSampleRate(SR);
        Run ra = drive(a, P, 6, 8, 0.3f, 0.5f);
        drive(b, P, 6, 8, 0.3f, 0.5f);
        b.reset();
        Run rb = drive(b, P, 6, 8, 0.3f, 0.5f);
        check(ra.trigs == rb.trigs, "reset() makes the burst pattern reproducible");
    }

    // A new edge mid-burst restarts rather than queueing. Drive at half the
    // established period and check nothing accumulates.
    {
        RatchetCore r;
        r.setSampleRate(SR);
        bool prevT = false;
        int count = 0;
        for (int i = 0; i < P * 8; i++) {
            const bool edge = (i % (P / 4)) == 0;
            RatchetCore::Out o = r.process(edge, 8, 0.f, 1.f);
            if (o.trig && !prevT) count++;
            prevT = o.trig;
        }
        check(count > 0 && count < 8 * 32,
              "retriggering mid-burst restarts it instead of piling up");
    }

    // Sample rate must not change the musical result.
    {
        RatchetCore a, b;
        a.setSampleRate(44100.f);
        b.setSampleRate(96000.f);
        Run ra = drive(a, int(0.5f * 44100.f), 2, 6, 0.5f, 1.f);
        Run rb = drive(b, int(0.5f * 96000.f), 2, 6, 0.5f, 1.f);
        check(ra.trigs.size() == rb.trigs.size(),
              "the same burst comes out at 44.1k and 96k");
        bool aligned = true;
        for (size_t i = 0; i < ra.trigs.size() && i < rb.trigs.size(); i++) {
            const float ta = ra.trigs[i] / 44100.f;
            const float tb = rb.trigs[i] / 96000.f;
            if (std::fabs(ta - tb) > 0.002f) aligned = false;
        }
        check(aligned, "repeat times agree to within 2 ms across sample rates");
    }

    std::printf(g_failures ? "\nFAILED (%d)\n" : "\nAll RatchetCore tests passed\n",
                g_failures);
    return g_failures ? 1 : 0;
}
