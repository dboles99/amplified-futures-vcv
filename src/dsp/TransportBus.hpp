#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// TransportBus — the message Street Grid Clock sends rightwards, and the
// rule deciding whether a module listens to it.
//
// Street Grid Clock is the only clock in the series, and every sequenced
// module needs the same edge from it: Pulse, Ratchet, Sitar Grid, Collapse EG
// and Drift's SYNC. On a full patch that is five cables carrying one signal.
// This lets an adjacent module inherit clock, reset and run without a cable.
//
// Deliberately has NO Rack dependency, so tests/test_transport_bus.cpp can
// exercise it with plain g++. The Rack side - allocating the two expander
// buffers, checking the neighbour's model before writing, requesting the flip
// - lives in src/AFExpander.hpp, which cannot be tested offline.
//
// One sample of latency is inherent: requestMessageFlip() swaps the buffers at
// the end of the timestep. That is exactly what a patch cable already costs in
// Rack, so a bussed clock and a patched clock arrive together. That
// equivalence is why the transport bus is the one worth building first, and
// why an audio-summing expander is not - there, one sample between channels is
// a comb filter rather than nothing.
// ============================================================
#include <cstddef>
#include <cstdint>

// Fixed-size POD, written every sample into a buffer the receiver owns.
// No allocation, no std::vector, no strings: this crosses the audio thread.
struct TransportMessage {
    float clock;      // 0 or 10 V — the same edge CLK carries
    float reset;      // 0 or 10 V
    float running;    // 0 or 10 V
    float bpm;        // for modules wanting tempo rather than edges
    uint32_t seq;     // advances once per sample the producer writes
    bool  valid;      // false when nothing upstream is driving the bus

    TransportMessage()
        : clock(0.f), reset(0.f), running(0.f), bpm(0.f), seq(0u), valid(false) {}
};

// The message is memcpy'd between modules, so it must stay trivially
// copyable. A std::string or a vector added to it later would compile and
// then corrupt across the expander boundary.
#if __cplusplus >= 201103L
#include <type_traits>
static_assert(std::is_trivially_copyable<TransportMessage>::value,
              "TransportMessage crosses the expander boundary by copy");
static_assert(std::is_standard_layout<TransportMessage>::value,
              "TransportMessage must be standard-layout POD");
#endif

// ─── The override rule ───────────────────────────────────────
//
// A patched input ALWAYS wins. The expander is a convenience, never the only
// route to a parameter - if the bus could override a cable, a module would
// behave differently depending on what happened to be sitting beside it, and
// the patch would stop describing itself.
//
// `patched` is the port's isConnected(), not whether it is currently high: an
// input patched to a silent source is still patched, and must not fall back to
// the bus between edges.
inline float transportPick(bool patched, float patchedVolts,
                           bool busValid, float busVolts) {
    if (patched)
        return patchedVolts;
    if (busValid)
        return busVolts;
    return 0.f;
}

// True when the module should take a CLOCK edge from the bus: something
// upstream is driving it AND the transport is running. A stopped clock holds
// its consumers stopped rather than letting them free-run, which is what the
// cable would do.
inline bool transportEngaged(const TransportMessage& bus) {
    return bus.valid && bus.running > 1.f;
}

// RESET is deliberately NOT gated on running.
//
// Street Grid Clock emits its RESET pulse whether or not the transport is
// running - doReset is independent of it - and aligning a sequence while
// stopped, then starting, is the ordinary way to use a reset. Gating the bus
// on running would mean a patched RESET cable worked stopped and the bus did
// not, which breaks the rule the whole design rests on: the bus behaves like
// the cable it replaces.
inline bool transportResetEngaged(const TransportMessage& bus) {
    return bus.valid;
}

// Has the producer actually written since the last sample?
//
// The model check alone is not enough. A module that is BYPASSED runs
// processBypass() instead of process(), so it stops writing while remaining a
// valid writer by model - and the receiver's consumerMessage then holds the
// last message forever. A clock frozen high would clock nothing; a clock
// frozen low would look like a stopped transport that cannot be restarted.
// Neither is detectable by inspecting the buffer's contents, so the producer
// stamps a sequence number and the receiver watches it advance.
inline bool transportAdvanced(bool seenBefore, uint32_t lastSeq,
                              const TransportMessage& msg) {
    return !seenBefore || msg.seq != lastSeq;
}

// What a module forwards to its right when it is passing the bus along.
// Chaining is the point: a row of modules should all inherit from the one
// clock at the left end, not just the module touching it.
//
// An invalid bus is forwarded as invalid rather than as zeros, so a gap in the
// row reads as "no transport here" downstream instead of as a stopped clock.
//
// Latency accumulates along the chain. Each hop is one flip, so the Nth module
// from the clock hears the edge N samples late - 68 us at three hops and
// 44.1 kHz, which is below the threshold for a percussive attack and far below
// the swing the clock is generating on purpose. It is stated rather than
// hidden because it is the reason this pattern suits clock and not audio: the
// same accumulation across a mixer expander is a comb filter, which is why the
// plan puts that one second and behind a design decision.
inline TransportMessage transportForward(const TransportMessage& incoming) {
    return incoming;
}
