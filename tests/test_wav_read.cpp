// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Standalone tests for the Swarm Core WAV reader. No Rack, no VCV SDK.
//
// Every fixture is synthesised here rather than committed, so the suite states
// exactly what shape of file it is describing and runs anywhere.
#include "../src/dsp/WavRead.hpp"
#include <cmath>
#include <cstdio>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

// --- fixture writing ------------------------------------------------------

static void put16(std::vector<uint8_t>& b, uint16_t v) {
    b.push_back(v & 0xff); b.push_back((v >> 8) & 0xff);
}
static void put32(std::vector<uint8_t>& b, uint32_t v) {
    for (int i = 0; i < 4; i++) b.push_back((v >> (8 * i)) & 0xff);
}
static void puts4(std::vector<uint8_t>& b, const char* s) {
    for (int i = 0; i < 4; i++) b.push_back((uint8_t) s[i]);
}

/// A WAV with an arbitrary fmt chunk and data payload, plus optional extra
/// chunks written before the data chunk.
static std::string writeWav(const char* name,
                            uint16_t fmtTag, uint16_t numCh, uint32_t rate,
                            uint16_t bits, const std::vector<uint8_t>& data,
                            const std::vector<uint8_t>& extraChunks
                                = std::vector<uint8_t>(),
                            bool extensibleFmt = false,
                            uint32_t declaredDataSize = 0xffffffffu) {
    std::vector<uint8_t> b;
    const uint16_t block = (uint16_t)(numCh * bits / 8);
    std::vector<uint8_t> fmt;
    put16(fmt, fmtTag); put16(fmt, numCh); put32(fmt, rate);
    put32(fmt, rate * block); put16(fmt, block); put16(fmt, bits);
    if (extensibleFmt) {
        // cbSize plus the 22-byte extension, whose GUID opens with the real
        // format tag.
        put16(fmt, 22);
        put16(fmt, bits);              // valid bits per sample
        put32(fmt, 0);                 // channel mask
        put16(fmt, 1);                 // SubFormat: PCM
        for (int i = 0; i < 14; i++) fmt.push_back(0);
    }

    puts4(b, "RIFF"); put32(b, 0); puts4(b, "WAVE");
    puts4(b, "fmt "); put32(b, (uint32_t) fmt.size());
    for (size_t i = 0; i < fmt.size(); i++) b.push_back(fmt[i]);
    if (fmt.size() & 1) b.push_back(0);
    for (size_t i = 0; i < extraChunks.size(); i++) b.push_back(extraChunks[i]);
    puts4(b, "data");
    put32(b, declaredDataSize == 0xffffffffu ? (uint32_t) data.size()
                                             : declaredDataSize);
    for (size_t i = 0; i < data.size(); i++) b.push_back(data[i]);
    // Patch the RIFF size now that it is known.
    uint32_t riff = (uint32_t)(b.size() - 8);
    for (int i = 0; i < 4; i++) b[4 + i] = (riff >> (8 * i)) & 0xff;

    FILE* f = fopen(name, "wb");
    fwrite(b.data(), 1, b.size(), f);
    fclose(f);
    return name;
}

/// 16-bit PCM payload: a ramp on channel 0 and a constant far from it on the
/// others, so channel selection and frame order are both visible in the values.
static std::vector<uint8_t> pcm16(int frames, int channels) {
    std::vector<uint8_t> d;
    for (int i = 0; i < frames; i++)
        for (int c = 0; c < channels; c++)
            put16(d, (uint16_t)(int16_t)((c == 0 ? 1000 : -30000) + i * 100));
    return d;
}

int main() {
    // Unbuffered: a malformed file can take the process down mid-suite, and a
    // buffered stdout throws away every result up to that point. The first run
    // of this suite died on a divide by zero and printed nothing at all.
    setvbuf(stdout, NULL, _IONBF, 0);

    std::vector<float> out;
    int sr = 0;

    // --- the cases that already worked ------------------------------------
    {
        std::string p = writeWav("t_16mono.wav", 1, 1, 44100, 16, pcm16(8, 1));
        check(WavRead::loadWavMono(p, out, sr), "16-bit mono PCM loads");
        check(sr == 44100, "sample rate is read from the file");
        check(out.size() == 8, "frame count matches the data chunk");
        check(std::fabs(out[0] - 1000.f / 32768.f) < 1e-6f,
              "16-bit values scale to full scale 1.0");
    }
    {
        std::string p = writeWav("t_16stereo.wav", 1, 2, 48000, 16, pcm16(8, 2));
        check(WavRead::loadWavMono(p, out, sr), "16-bit stereo PCM loads");
        check(out.size() == 8, "stereo gives one value per frame, not per sample");
        check(out[1] > 0.f, "channel 0 is taken, not interleaved garbage");
    }
    {
        std::vector<uint8_t> d;
        for (int i = 0; i < 4; i++) {
            float v = 0.25f * (i + 1);
            uint8_t raw[4];
            std::memcpy(raw, &v, 4);
            for (int k = 0; k < 4; k++) d.push_back(raw[k]);
        }
        std::string p = writeWav("t_f32.wav", 3, 1, 44100, 32, d);
        check(WavRead::loadWavMono(p, out, sr), "32-bit float loads");
        check(std::fabs(out[3] - 1.0f) < 1e-6f,
              "float values pass through unscaled");
    }

    // --- WAVE_FORMAT_EXTENSIBLE: 60 of the 670 InsectSet32 files ----------
    {
        std::string p = writeWav("t_ext16.wav", 0xFFFE, 1, 44100, 16,
                                 pcm16(8, 1), std::vector<uint8_t>(), true);
        check(WavRead::loadWavMono(p, out, sr),
              "WAVE_FORMAT_EXTENSIBLE wrapping PCM is not rejected");
        check(out.size() == 8, "extensible PCM decodes to the right length");
        check(std::fabs(out[0] - 1000.f / 32768.f) < 1e-6f,
              "extensible PCM decodes to the right values");
    }

    // --- bit depths the reader did not actually handle ---------------------
    // 24-bit fell through to the 16-bit branch, so it produced noise rather
    // than failing. Silently wrong is worse than rejected.
    {
        std::vector<uint8_t> d;
        for (int i = 0; i < 4; i++) {              // ascending, 0.25 up to 1.0
            int32_t v = (int32_t)((0.25 * (i + 1)) * 8388607.0);
            d.push_back(v & 0xff);
            d.push_back((v >> 8) & 0xff);
            d.push_back((v >> 16) & 0xff);
        }
        std::string p = writeWav("t_24.wav", 1, 1, 44100, 24, d);
        check(WavRead::loadWavMono(p, out, sr), "24-bit PCM loads");
        check(out.size() == 4, "24-bit frame count uses 3 bytes per sample");
        check(std::fabs(out[3] - 1.0f) < 1e-3f, "24-bit values scale to 1.0");
    }
    {
        std::vector<uint8_t> d;
        d.push_back(128); d.push_back(192); d.push_back(255); d.push_back(64);
        std::string p = writeWav("t_8.wav", 1, 1, 22050, 8, d);
        check(WavRead::loadWavMono(p, out, sr), "8-bit PCM loads");
        check(out.size() == 4, "8-bit frame count uses 1 byte per sample");
        check(std::fabs(out[0]) < 1e-6f, "8-bit 128 is silence, not full scale");
        check(out[1] > 0.4f && out[3] < -0.4f, "8-bit is unsigned-centred");
    }

    // --- chunk walking -----------------------------------------------------
    // RIFF pads odd chunks to an even length. Skipping only the declared size
    // leaves the reader one byte out of step, and it never finds the data.
    {
        std::vector<uint8_t> extra;
        puts4(extra, "LIST"); put32(extra, 5);
        for (int i = 0; i < 5; i++) extra.push_back('x');
        extra.push_back(0);                          // the pad byte
        std::string p = writeWav("t_oddchunk.wav", 1, 1, 44100, 16,
                                 pcm16(8, 1), extra);
        check(WavRead::loadWavMono(p, out, sr),
              "an odd-sized chunk before data is skipped with its pad byte");
        check(out.size() == 8, "data after an odd chunk reads at full length");
    }

    // --- malformed input must fail, not crash or invent samples ------------
    {
        std::string p = writeWav("t_zeroch.wav", 1, 0, 44100, 16, pcm16(4, 1));
        check(!WavRead::loadWavMono(p, out, sr),
              "zero channels is rejected, not divided by");
    }
    {
        // Declares far more data than the file holds: a truncated download.
        std::string p = writeWav("t_short.wav", 1, 1, 44100, 16, pcm16(4, 1),
                                 std::vector<uint8_t>(), false, 4096);
        bool ok = WavRead::loadWavMono(p, out, sr);
        check(!ok || out.size() == 4,
              "a truncated data chunk yields what exists, never zero padding");
    }
    {
        FILE* f = fopen("t_notwav.wav", "wb");
        fputs("not a wav file at all", f);
        fclose(f);
        check(!WavRead::loadWavMono("t_notwav.wav", out, sr),
              "a non-WAV is rejected");
    }

    // --- truncation, which is the whole contract with the module -----------
    {
        std::string p = writeWav("t_long.wav", 1, 1, 44100, 16, pcm16(100, 1));
        check(WavRead::loadWavMono(p, out, sr, 32), "maxFrames caps the read");
        check(out.size() == 32, "maxFrames caps at exactly maxFrames");
    }

    std::printf("\n%s\n", g_failures ? "FAILURES" : "all pass");
    return g_failures ? 1 : 0;
}
