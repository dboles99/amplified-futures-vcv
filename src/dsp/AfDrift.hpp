#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// AfDrift — Per-voice pitch drift with an explicit coherence axis.
//
// Coherence is the parameter usually left out, and it is the one that decides
// what the effect is. At 0 every voice shares one drift signal: ratios hold
// exactly and the whole instrument transposes. At 1 the voices drift
// independently: ratios hold only on average, spectral fusion breaks down, and
// the stack becomes a chorus. Twenty violins, a hundred guitars and a drifting
// Moog are all the second case.
//
// No Rack dependency, in the same shape as ClockCore, EnvCore, VcaCore,
// RatchetCore and BlocCore.
// ============================================================

#include "AfTuning.hpp"

#include <cmath>
#include <cstdint>

namespace af::tuning {

class Drift {
public:
    static constexpr int kMaxVoices = 16;

    void reset(int voices, uint32_t seed) noexcept
    {
        voices_ = voices < 1 ? 1 : (voices > kMaxVoices ? kMaxVoices : voices);
        // Phases are staggered rather than zeroed: identical starting phase
        // across voices produces a coherence-1 stack that is momentarily
        // coherent, which is an audible startup artefact. Same reasoning as
        // the oscillator phase stagger elsewhere in this plugin.
        for (int v = 0; v < kMaxVoices; ++v) {
            phase_[v] = static_cast<float>(v) / static_cast<float>(kMaxVoices);
            // A cheap deterministic per-voice rate offset, so voices do not
            // converge into a single beat. Deterministic because presets must
            // recall identically.
            uint32_t h = seed + 0x9E3779B9u * static_cast<uint32_t>(v + 1);
            h ^= h >> 16; h *= 0x7FEB352Du; h ^= h >> 15;
            rateMul_[v] = 0.75f + 0.5f * (static_cast<float>(h & 0xFFFFu) / 65535.f);
            out_[v] = 0.f;
        }
        shared_ = 0.f;
        sharedPhase_ = 0.f;
    }

    void setRate(float hz) noexcept
    {
        if (std::isfinite(hz) && hz >= 0.f) rate_ = hz;
    }
    void setDepth(float cents) noexcept
    {
        if (std::isfinite(cents) && cents >= 0.f) depth_ = cents;
    }
    void setCoherence(float c) noexcept
    {
        if (std::isfinite(c)) {
            coherence_ = c < 0.f ? 0.f : (c > 1.f ? 1.f : c);
        }
    }

    void process(float sampleTime) noexcept
    {
        // A single shared oscillator drives the coherent component.
        sharedPhase_ += rate_ * sampleTime;
        if (sharedPhase_ >= 1.f) sharedPhase_ -= std::floor(sharedPhase_);
        shared_ = std::sin(sharedPhase_ * 6.283185307f);

        for (int v = 0; v < voices_; ++v) {
            phase_[v] += rate_ * rateMul_[v] * sampleTime;
            if (phase_[v] >= 1.f) phase_[v] -= std::floor(phase_[v]);
            const float indep = std::sin(phase_[v] * 6.283185307f);

            // Crossfade, so depth is a hard bound at either extreme and
            // anywhere between. Both terms are in [-1, 1].
            const float mixed = shared_ * (1.f - coherence_) + indep * coherence_;
            out_[v] = mixed * depth_;
        }
    }

    [[nodiscard]] float centsFor(int voice) const noexcept
    {
        if (voice < 0 || voice >= voices_) return 0.f;
        return out_[voice];
    }

private:
    int   voices_ = 1;
    float rate_ = 0.f;
    float depth_ = 0.f;
    float coherence_ = 1.f;
    float sharedPhase_ = 0.f;
    float shared_ = 0.f;
    float phase_[kMaxVoices] {};
    float rateMul_[kMaxVoices] {};
    float out_[kMaxVoices] {};
};

} // namespace af::tuning
