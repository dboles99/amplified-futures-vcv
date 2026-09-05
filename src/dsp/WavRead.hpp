// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Minimal WAV reader for Swarm Core's sample bank. No Rack dependency, so it
// can be tested offline against synthesised files.
//
// Reads the first channel only and stops at maxFrames: the module is a swarm
// of short one-shots, not a player, and every sample it holds ends up mono and
// truncated regardless.
//
// Covers PCM at 8, 16, 24 and 32 bits and IEEE float at 32 and 64, in both
// plain WAVE and WAVE_FORMAT_EXTENSIBLE. The earlier version accepted only
// tags 1 and 3 and assumed 16-bit for everything that was not 32-bit float, so
// it rejected 60 of the 670 InsectSet32 recordings outright and decoded 24-bit
// files into noise without reporting anything wrong.
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace WavRead {

namespace detail {

inline uint16_t le16(const uint8_t* b) {
    return (uint16_t)(b[0] | (b[1] << 8));
}
inline uint32_t le32(const uint8_t* b) {
    return (uint32_t) b[0] | ((uint32_t) b[1] << 8)
         | ((uint32_t) b[2] << 16) | ((uint32_t) b[3] << 24);
}

/// One frame of channel 0, as a float in roughly -1..1.
inline float decode(const uint8_t* p, uint16_t fmtTag, uint16_t bits) {
    if (fmtTag == 3) {                                  // IEEE float
        if (bits == 32) { float v;  std::memcpy(&v, p, 4); return v; }
        double v; std::memcpy(&v, p, 8); return (float) v;
    }
    switch (bits) {                                     // PCM
        case 8:  return ((int) p[0] - 128) / 128.f;     // unsigned, 128 is zero
        case 16: return (int16_t) le16(p) / 32768.f;
        case 24: {
            int32_t v = (int32_t)(((uint32_t) p[0] << 8)
                                | ((uint32_t) p[1] << 16)
                                | ((uint32_t) p[2] << 24));
            return (v >> 8) / 8388608.f;                // sign-extend, then scale
        }
        default: return (int32_t) le32(p) / 2147483648.f;
    }
}

} // namespace detail

/// Read `path` into `out` as mono float, at most `maxFrames` frames.
/// Returns false and leaves `out` empty if the file cannot be decoded.
inline bool loadWavMono(const std::string& path,
                        std::vector<float>& out, int& sampleRate,
                        uint32_t maxFrames = 220500) { // 5 s @ 44.1 kHz
    out.clear();
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;

    uint8_t hdr[12];
    if (fread(hdr, 1, 12, f) != 12
        || memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) {
        fclose(f);
        return false;
    }

    uint16_t fmtTag = 0, numCh = 0, bits = 0;
    uint32_t rateHz = 44100, dataSize = 0;
    bool haveFmt = false;

    for (;;) {
        uint8_t ch[8];
        if (fread(ch, 1, 8, f) != 8) break;             // ran out of chunks
        const uint32_t sz = detail::le32(ch + 4);

        if (memcmp(ch, "fmt ", 4) == 0 && sz >= 16) {
            std::vector<uint8_t> fmt(sz);
            if (fread(fmt.data(), 1, sz, f) != sz) break;
            fmtTag = detail::le16(&fmt[0]);
            numCh  = detail::le16(&fmt[2]);
            rateHz = detail::le32(&fmt[4]);
            bits   = detail::le16(&fmt[14]);
            // WAVE_FORMAT_EXTENSIBLE carries the real tag in the first two
            // bytes of the SubFormat GUID, 8 bytes into the 22-byte extension.
            if (fmtTag == 0xFFFE && sz >= 40)
                fmtTag = detail::le16(&fmt[24]);
            haveFmt = true;
            if (sz & 1) fseek(f, 1, SEEK_CUR);          // RIFF pads to even
        }
        else if (memcmp(ch, "data", 4) == 0) {
            dataSize = sz;
            break;                                      // read it in place
        }
        else {
            // The pad byte belongs to the chunk, not to the next one. Skipping
            // only the declared size leaves every later chunk one byte out of
            // step, so `data` is never found in a file carrying odd metadata.
            if (fseek(f, (long)(sz + (sz & 1)), SEEK_CUR) != 0) break;
        }
    }

    const uint16_t bytesPerSample = (uint16_t)(bits / 8);
    const bool formatOk = haveFmt
        && (fmtTag == 1 || fmtTag == 3)
        && numCh > 0
        && (fmtTag == 1 ? (bits == 8 || bits == 16 || bits == 24 || bits == 32)
                        : (bits == 32 || bits == 64));
    if (!formatOk || dataSize == 0) {
        fclose(f);
        return false;
    }

    sampleRate = (int) rateHz;
    const uint32_t blockAlign = (uint32_t) numCh * bytesPerSample;
    uint32_t frames = std::min(dataSize / blockAlign, maxFrames);

    std::vector<uint8_t> raw((size_t) frames * blockAlign);
    // A file can declare more data than it holds. Size the result by what was
    // actually read, so a truncated download yields a short sample rather than
    // a tail of silence.
    const size_t got = fread(raw.data(), 1, raw.size(), f);
    fclose(f);
    frames = (uint32_t)(got / blockAlign);

    out.resize(frames);
    for (uint32_t i = 0; i < frames; i++)
        out[i] = detail::decode(&raw[(size_t) i * blockAlign], fmtTag, bits);
    return !out.empty();
}

} // namespace WavRead
