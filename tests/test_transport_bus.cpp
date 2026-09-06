// Standalone tests for TransportBus. No Rack, no VCV SDK - plain g++.
//
// The expander plumbing in src/AFExpander.hpp cannot be tested here: it needs
// a Module, a neighbour and an engine to flip the buffers. What CAN be tested
// is the part that decides behaviour - the override rule, whether the bus is
// engaged, and that the message stays a POD safe to copy across the boundary.
// Those are the three things that would be wrong quietly.
#include "../src/dsp/TransportBus.hpp"
#include <cstdio>
#include <type_traits>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

static TransportMessage running_bus(float clock = 10.f) {
    TransportMessage m;
    m.clock = clock;
    m.reset = 0.f;
    m.running = 10.f;
    m.bpm = 120.f;
    m.valid = true;
    return m;
}

int main() {
    std::printf("== TransportBus\n");

    // ── The default is silence, not a stuck clock ───────────
    {
        TransportMessage m;
        check(!m.valid, "a default message is invalid");
        check(m.clock == 0.f && m.reset == 0.f, "a default message carries no edge");
        check(m.running == 0.f, "a default message is not running");
    }

    // ── A patched input always wins ─────────────────────────
    //
    // Gate 4 of the plan: the expander is never the only way to reach a
    // parameter. If the bus could override a cable, the same patch would
    // behave differently depending on what sat beside it.
    {
        TransportMessage bus = running_bus(10.f);
        check(transportPick(true, 0.f, transportEngaged(bus), bus.clock) == 0.f,
              "a patched input at 0 V beats a bus carrying 10 V");
        check(transportPick(true, 5.f, transportEngaged(bus), bus.clock) == 5.f,
              "a patched input passes its own voltage through");
        check(transportPick(false, 0.f, transportEngaged(bus), bus.clock) == 10.f,
              "an unpatched input takes the bus");
    }

    // A cable from a silent source is still a cable. This is the case that
    // would look correct in casual testing: the module would fall back to the
    // bus between edges and appear to work, while ignoring the patch.
    {
        TransportMessage bus = running_bus(10.f);
        check(transportPick(true, 0.f, transportEngaged(bus), bus.clock) == 0.f,
              "a patched but idle input does not fall through to the bus");
    }

    // ── No bus, no signal ───────────────────────────────────
    {
        TransportMessage none;   // invalid
        check(transportPick(false, 0.f, transportEngaged(none), none.clock) == 0.f,
              "an unpatched input with no bus reads 0 V");
    }

    // ── A stopped clock stops its consumers ─────────────────
    {
        TransportMessage bus = running_bus(10.f);
        bus.running = 0.f;
        check(!transportEngaged(bus), "a stopped transport is not engaged");
        check(transportPick(false, 0.f, transportEngaged(bus), bus.clock) == 0.f,
              "a stopped transport delivers no edge");
    }

    {
        TransportMessage bus = running_bus(10.f);
        bus.valid = false;       // producer vanished mid-patch
        check(!transportEngaged(bus), "an invalid message is never engaged");
        check(transportPick(false, 0.f, transportEngaged(bus), bus.clock) == 0.f,
              "a vanished producer delivers no edge, even holding a stale 10 V");
    }

    // ── Reset is not gated on running ───────────────────────
    //
    // Street Grid Clock emits RESET whether or not it is running, and aligning
    // a sequence while stopped then starting is the ordinary way to use it. A
    // patched cable works stopped; the bus has to as well, or the bus stops
    // behaving like the cable it replaces.
    {
        TransportMessage bus = running_bus(0.f);
        bus.running = 0.f;
        bus.reset = 10.f;
        check(!transportEngaged(bus), "a stopped transport gives no clock");
        check(transportResetEngaged(bus), "a stopped transport still gives reset");
        check(transportPick(false, 0.f, transportResetEngaged(bus), bus.reset) == 10.f,
              "an unpatched reset fires from a stopped transport");
    }

    {
        TransportMessage none;
        check(!transportResetEngaged(none),
              "reset from an absent transport is still nothing");
    }

    {
        TransportMessage bus = running_bus(0.f);
        bus.running = 0.f;
        bus.reset = 10.f;
        check(transportPick(true, 0.f, transportResetEngaged(bus), bus.reset) == 0.f,
              "a patched reset still beats the bus while stopped");
    }

    // ── A producer that stops writing ───────────────────────
    //
    // A BYPASSED module runs processBypass() instead of process(), so it stops
    // writing while still being a valid writer by model. Its last message sits
    // in the receiver's buffer unchanged - a clock frozen high or low, neither
    // distinguishable from a live one by looking at the contents. The sequence
    // number is what separates them.
    {
        TransportMessage msg = running_bus(10.f);
        msg.seq = 41;
        check(transportAdvanced(false, 0, msg),
              "the first message ever seen counts as advanced");
        check(transportAdvanced(true, 40, msg),
              "a message with a new sequence is live");
        check(!transportAdvanced(true, 41, msg),
              "a message repeating its sequence is a producer that stopped");
    }

    // The counter wrapping must not read as a stall: the values differ either
    // side of the wrap, which is all the check asks.
    {
        TransportMessage msg = running_bus(10.f);
        msg.seq = 0;
        check(transportAdvanced(true, 0xFFFFFFFFu, msg),
              "a wrapped sequence still counts as advanced");
    }

    // ── Forwarding ──────────────────────────────────────────
    {
        TransportMessage bus = running_bus(10.f);
        bus.reset = 10.f;
        TransportMessage fwd = transportForward(bus);
        check(fwd.clock == bus.clock && fwd.reset == bus.reset
              && fwd.running == bus.running && fwd.bpm == bus.bpm
              && fwd.valid == bus.valid,
              "forwarding passes every field through unchanged");
    }

    // A gap in the row must read as "no transport" downstream, not as a
    // stopped clock - those are different, and only one of them is true.
    {
        TransportMessage none;
        check(!transportForward(none).valid,
              "forwarding an absent bus stays absent rather than becoming stopped");
    }

    // ── The message must stay copyable ──────────────────────
    //
    // Enforced by static_assert in the header, so a std::string added later
    // fails the build rather than corrupting across the boundary. Checked
    // here too so the reason is written down where it is tested.
    {
        TransportMessage a = running_bus(7.f);
        TransportMessage b = a;             // the copy the expander performs
        check(b.clock == 7.f && b.valid, "a copied message keeps its contents");
        // Not sizeof(T) == sizeof(a), which is true by definition and tests
        // nothing. These are the properties that actually make the copy above
        // safe across the expander boundary.
        check(std::is_trivially_copyable<TransportMessage>::value,
              "the message is trivially copyable");
        check(std::is_standard_layout<TransportMessage>::value,
              "the message is standard-layout");
    }

    std::printf("\n%s\n", g_failures ? "FAILURES" : "all passed");
    return g_failures ? 1 : 0;
}
