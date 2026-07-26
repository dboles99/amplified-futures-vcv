// Standalone tests for ClockCore. No Rack, no VCV SDK - plain g++.
//
// VCV modules in this repo have had no automated test of any kind. ClockCore
// deliberately has no Rack dependency precisely so this file can exist.
#include "../src/dsp/ClockCore.hpp"
#include <cmath>
#include <cstdio>
#include <vector>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

// Run the core for `seconds` and return the sample index of every CLK rising edge.
static std::vector<int> clkEdges(ClockCore& c, float sr, float seconds,
                                 float bpm, float swing, float brownout) {
    std::vector<int> edges;
    const int n = static_cast<int>(sr * seconds);
    for (int i = 0; i < n; ++i) {
        ClockCore::Ticks t = c.process(bpm, swing, brownout, true);
        if (t.clk) edges.push_back(i);
    }
    return edges;
}

int main() {
    const float SR = 48000.f;

    // 120 BPM = 2 quarter-notes per second = a 0.5 s period.
    {
        ClockCore c; c.setSampleRate(SR); c.reset();
        std::vector<int> e = clkEdges(c, SR, 4.f, 120.f, 0.f, 0.f);
        check(e.size() >= 7 && e.size() <= 9, "120 BPM gives ~8 pulses in 4 s");
        bool spacingOk = true;
        for (size_t i = 1; i < e.size(); ++i) {
            int d = e[i] - e[i - 1];
            if (std::abs(d - 24000) > 2) spacingOk = false;  // 0.5 s +/- 2 samples
        }
        check(spacingOk, "120 BPM pulse spacing is 24000 samples");
    }

    // Divisions must be exact ratios of the base clock.
    {
        ClockCore c; c.setSampleRate(SR); c.reset();
        int clk = 0, d2 = 0, d4 = 0, d8 = 0;
        for (int i = 0; i < static_cast<int>(SR * 8.f); ++i) {
            ClockCore::Ticks t = c.process(240.f, 0.f, 0.f, true);
            clk += t.clk; d2 += t.div2; d4 += t.div4; d8 += t.div8;
        }
        check(clk > 0, "base clock fires");
        check(std::abs(d2 * 2 - clk) <= 2, "div2 fires half as often as clk");
        check(std::abs(d4 * 4 - clk) <= 4, "div4 fires a quarter as often");
        check(std::abs(d8 * 8 - clk) <= 8, "div8 fires an eighth as often");
    }

    // Not running: no output at all.
    {
        ClockCore c; c.setSampleRate(SR); c.reset();
        int fired = 0;
        for (int i = 0; i < 48000; ++i)
            fired += c.process(120.f, 0.f, 0.f, false).clk;
        check(fired == 0, "stopped clock emits nothing");
    }

    // reset() returns to a known state, so two runs match exactly.
    {
        ClockCore a, b;
        a.setSampleRate(SR); b.setSampleRate(SR);
        a.reset(); b.reset();
        std::vector<int> ea = clkEdges(a, SR, 2.f, 137.f, 0.f, 0.f);
        std::vector<int> eb = clkEdges(b, SR, 2.f, 137.f, 0.f, 0.f);
        check(ea == eb, "reset gives reproducible timing");
    }

    std::printf(g_failures ? "\nFAILED (%d)\n" : "\nAll ClockCore tests passed\n",
                g_failures);
    return g_failures ? 1 : 0;
}
