#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// SitarStringCore - the three delay lines behind Sitar Grid.
//
// Extracted from SitarGrid.cpp so the string model can be driven by a
// standalone binary (tests/test_sitar_string.cpp), as with ClockCore and
// the other AF cores. No Rack dependency.
//
// The extraction was forced by a defect no test could have caught while
// the model lived inline: a plucked string that returned silence forever.
// See the header comment on PluckedString::pluck.
//
// Randomness is owned here rather than taken from rack::random, both to
// drop the dependency and so a test can seed it and get the same burst
// twice.
// ============================================================
#include <cmath>
#include <cstdint>

namespace SitarStringCore {

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/// xorshift32. Allocation-free and deterministic from a seed, which is what
/// an audio thread and a test respectively need.
class Rng {
public:
    explicit Rng(uint32_t seed = 0x9E3779B9u) : state(seed ? seed : 1u) {}

    /// Uniform in [0, 1).
    float uniform() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return (state >> 8) * (1.0f / 16777216.0f);
    }

    /// Uniform in [-1, 1).
    float bipolar() { return uniform() * 2.f - 1.f; }

private:
    uint32_t state;
};

/// Delay length for a pitch, in samples. Shared by every string so that a
/// pluck and the ticks that follow it agree on where the burst lives.
inline int delayLength(float freq, float sr, int maxLen) {
    return clampi((int)(sr / (freq > 20.f ? freq : 20.f)), 2, maxLen - 1);
}

// ------------------------------------------------------------
// Main string - Karplus-Strong with a lowpass-filtered noise burst.
// ------------------------------------------------------------
class PluckedString {
public:
    static const int MAX_LEN = 4096;

    /// Excite the string.
    ///
    /// The burst must land in [wptr - len, wptr) - the span the read head
    /// traverses over the next `len` samples. Writing it to [wptr, wptr+len)
    /// instead puts it a full lap ahead of the read head, and the write head
    /// then overwrites every sample of it with filtered silence before the
    /// read head ever arrives. That is what made every audio output of Sitar
    /// Grid read exactly 0.0 V in each of its shipped builds.
    void pluck(float freq, float vel, float brightness, float sr, Rng& rng) {
        const int len = delayLength(freq, sr, MAX_LEN);
        const float alpha = clampf(0.15f + brightness * 0.82f, 0.01f, 1.f);
        float lp = 0.f;
        for (int i = 0; i < len; i++) {
            lp += alpha * (rng.bipolar() - lp);
            buf[((wptr - len + i) % MAX_LEN + MAX_LEN) % MAX_LEN] = lp * vel;
        }
    }

    /// One sample. `damping` 0..1 sets the loop gain, which is applied once
    /// per traversal of the delay line - not once per sample - so it reads
    /// as ring time: roughly 9 s at 0, a quarter of a second at 0.3.
    float tick(float freq, float damping, float sr) {
        const int len = delayLength(freq, sr, MAX_LEN);
        const int r1  = ((wptr - len) % MAX_LEN + MAX_LEN) % MAX_LEN;
        const int r2  = (r1 + 1) % MAX_LEN;
        const float out = buf[r1];
        // Two-point averaging filter = built-in string damping.
        buf[wptr] = (out + buf[r2]) * 0.5f * (0.997f - damping * 0.28f);
        wptr = (wptr + 1) % MAX_LEN;
        return out;
    }

    void reset() {
        for (int i = 0; i < MAX_LEN; i++) buf[i] = 0.f;
        wptr = 0;
    }

private:
    float buf[MAX_LEN] = {};
    int   wptr = 0;
};

// ------------------------------------------------------------
// Chikari - the high drone string. Unfiltered burst, fixed loop gain.
// ------------------------------------------------------------
class DroneString {
public:
    static const int MAX_LEN = 4096;

    /// Same span rule as PluckedString::pluck, and the same defect history.
    void pluck(float freq, float vel, float sr, Rng& rng) {
        const int len = delayLength(freq, sr, MAX_LEN);
        for (int i = 0; i < len; i++)
            buf[((wptr - len + i) % MAX_LEN + MAX_LEN) % MAX_LEN] = rng.bipolar() * vel;
    }

    float tick(float freq, float sr) {
        const int len = delayLength(freq, sr, MAX_LEN);
        const int r1  = ((wptr - len) % MAX_LEN + MAX_LEN) % MAX_LEN;
        const int r2  = (r1 + 1) % MAX_LEN;
        const float out = buf[r1];
        buf[wptr] = (out + buf[r2]) * 0.5f * 0.992f;
        wptr = (wptr + 1) % MAX_LEN;
        return out;
    }

    void reset() {
        for (int i = 0; i < MAX_LEN; i++) buf[i] = 0.f;
        wptr = 0;
    }

private:
    float buf[MAX_LEN] = {};
    int   wptr = 0;
};

// ------------------------------------------------------------
// Sympathetic strings - eight comb filters driven by the main string.
// Never plucked directly; they ring only from what the main string feeds
// them, which is why they were silent too.
// ------------------------------------------------------------
class SympatheticBank {
public:
    static const int COUNT   = 8;
    static const int MAX_LEN = 2048;

    float tick(int s, float freq, float fbAmt, float input, float sr) {
        const int len = delayLength(freq, sr, MAX_LEN);
        const int r   = ((wptr[s] - len) % MAX_LEN + MAX_LEN) % MAX_LEN;
        const float out = buf[s][r];
        buf[s][wptr[s]] = input * 0.04f + out * fbAmt;
        wptr[s] = (wptr[s] + 1) % MAX_LEN;
        return out;
    }

    void reset() {
        for (int s = 0; s < COUNT; s++) {
            for (int i = 0; i < MAX_LEN; i++) buf[s][i] = 0.f;
            wptr[s] = 0;
        }
    }

private:
    float buf[COUNT][MAX_LEN] = {};
    int   wptr[COUNT] = {};
};

} // namespace SitarStringCore
