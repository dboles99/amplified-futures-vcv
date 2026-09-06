#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// AFExpander — the Rack side of the transport bus.
//
// The logic lives in dsp/TransportBus.hpp, which has no Rack dependency and is
// unit-tested. This file is the plumbing that cannot be: expander buffers,
// model checks and the message flip.
//
// The SDK's rules, read from include/engine/Module.hpp rather than recalled:
//
//   · Both buffers must be identical blocks of memory, owned for the module's
//     lifetime. Here they are a two-element array member, so allocation and
//     lifetime are the module's and there is nothing to free.
//   · "You must check the expander module's model before attempting to write
//     its message buffer." A neighbour of another brand has a buffer of some
//     other type and some other size; writing our struct into it is a stray
//     write into another plugin's memory.
//   · requestMessageFlip() swaps the buffers at the end of the timestep, so
//     the message arrives one sample later.
//
// The direction is the part that is easy to get backwards: the RECEIVER owns
// both buffers on the side facing the sender. A producer writes into its right
// neighbour's leftExpander.producerMessage and asks that neighbour to flip.
// ============================================================
#include <rack.hpp>
#include "dsp/TransportBus.hpp"

using namespace rack;

// Does this model read a transport bus arriving from its left - that is, does
// it own TransportMessage buffers on its left side? Defined in plugin.cpp,
// where every Model in the plugin is visible.
bool afReadsTransport(engine::Model* model);

// Does this model drive a transport bus rightwards? Street Grid Clock
// originates one; every reader also forwards, so a row chains from the clock
// at its left end.
bool afWritesTransport(engine::Model* model);


// Owned by any module that reads the bus. Declare one as a member and call
// init() in the constructor.
struct TransportReceiver {
    // Both buffers, same type and size, alive as long as the module.
    TransportMessage buf[2];

    void init(Module* m) {
        m->leftExpander.producerMessage = &buf[0];
        m->leftExpander.consumerMessage = &buf[1];
    }

    // What arrived this sample, or an invalid message if nothing upstream is
    // driving the bus.
    //
    // The staleness check is the whole job. After a flip, consumerMessage
    // holds whatever was last written - and it goes on holding it after the
    // neighbour is deleted, so a module that trusted the buffer alone would
    // keep clocking from a module that is no longer there. The left
    // neighbour's model is therefore re-checked every sample, not cached at
    // onExpanderChange: cheap, and it cannot go stale.
    TransportMessage read(Module* m) const {
        const Module* left = m->leftExpander.module;
        if (!left || !afWritesTransport(left->model))
            return TransportMessage();
        const void* src = m->leftExpander.consumerMessage;
        if (!src)
            return TransportMessage();
        return *reinterpret_cast<const TransportMessage*>(src);
    }
};


// Send `msg` to the right neighbour, if it is one of ours and is listening.
// Safe to call every sample with no neighbour present.
inline void transportSendRight(Module* m, const TransportMessage& msg) {
    Module* right = m->rightExpander.module;
    if (!right)
        return;
    // The model check the SDK requires, before any write.
    if (!afReadsTransport(right->model))
        return;
    void* dst = right->leftExpander.producerMessage;
    if (!dst)
        return;
    *reinterpret_cast<TransportMessage*>(dst) = msg;
    right->leftExpander.requestMessageFlip();
}
