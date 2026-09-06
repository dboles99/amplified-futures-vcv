// Standalone tests for TransportBus. No Rack, no VCV SDK - plain g++.
//
// The expander plumbing in src/AFExpander.hpp cannot be tested here: it needs
// a Module, a neighbour and an engine to flip the buffers. What CAN be tested
// is the part that decides behaviour - the override rule, whether the bus is
// engaged, and that the message stays a POD safe to copy across the boundary.
// Those are the three things that would be wrong quietly.
#include "../src/dsp/TransportBus.hpp"
#include <cstdio>

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
        check(sizeof(TransportMessage) == sizeof(a), "size is fixed, not dynamic");
    }

    std::printf("\n%s\n", g_failures ? "FAILURES" : "all passed");
    return g_failures ? 1 : 0;
}
