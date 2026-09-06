# Test patches

Patches that exist to check something, not to make music. Load one, follow its
steps, and it either does what it says or it does not.

## transport-bus-check.vcv

Checks the transport bus (`src/dsp/TransportBus.hpp`, `src/AFExpander.hpp`)
against the gates its plan set out. Everything here needs a running engine, so
none of it is covered by CI — CI proves the logic and the build, and cannot
prove any of the four checks below.

**Layout.** One row, five modules, touching:

    StreetGridClock | Pulse | Ratchet | Drift | CollapseEG

They must stay adjacent. The bus travels between neighbours, so a gap anywhere
breaks the chain from that point rightwards — which is itself worth seeing once.

**The only clock cable in the patch is deliberate.** Street Grid Clock's `/8`
output is patched to Pulse's `TRG`. Pulse is adjacent to the clock and so also
receives the bus, which carries `CLK`. The cable should win.

---

### 1 · Modules clock with no cable

Press RUN. Ratchet, Drift and Collapse EG should all respond to the clock
though nothing is patched to any of them.

*Failing looks like:* silence, or only the module directly beside the clock
responding. The second means forwarding is broken while receiving works.

### 2 · A patched input beats the bus

Pulse is fed `/8`. It should fire once for every eight of Ratchet's triggers.

*Failing looks like:* Pulse and Ratchet firing together, which means the bus is
overriding the cable — the one thing the design must never do.

### 3 · Removing the producer stops the row, and does not glitch

Delete Street Grid Clock while it is running. Everything downstream should
stop cleanly.

*Failing looks like:* a stuck note, a clock that keeps running from a module
that no longer exists, or a crash. The buffer holds its last message after the
producer is gone, so this is the case a naive reader gets wrong.

### 4 · Bypassing a module in the middle does not freeze the row

Bypass Ratchet (right-click → Bypass). Drift and Collapse EG, to its right,
should carry on.

*Failing looks like:* everything right of Ratchet freezing or holding a note.
A bypassed module runs `processBypass()` instead of `process()`, so it stops
forwarding unless it is explicitly written to. This case was found in review,
not by testing — worth confirming by hand for that reason.

---

### Also worth trying

- **Move a module out of the row** and back. The bus should drop and recover.
- **Patch a cable into Ratchet's TRIG** as well. Same rule as check 2.
- **Stop the transport** (RUN off) and press RESET. Sitar Grid takes reset from
  the bus while stopped, deliberately — the clock emits RESET when stopped, and
  a patched cable would work, so the bus has to as well.

### If something fails

The logic is unit-tested in `tests/test_transport_bus.cpp` — 27 assertions, and
they pass. A failure here is therefore in the plumbing rather than the rules:
`AFExpander.hpp`, the `processBypass` overrides, or the ordering of the read and
the forward inside `process()`.
