
---

## SECURITY.md

```markdown
# Security Policy

## Supported versions

| Version | Supported |
|---------|-----------|
| 1.0.x   | Yes       |

Only the current release receives fixes.

## Scope

Amplified Futures is a VCV Rack audio plugin. It processes audio and CV signals
in memory, has no network access, performs no file I/O beyond what Rack itself
handles, and ships as a compiled DLL loaded by Rack.

**In scope:** memory safety bugs in DSP code (buffer overruns, unchecked array
indices, undefined behaviour that could be triggered by a crafted patch).

**Out of scope:** vulnerabilities in VCV Rack itself — report those to VCV at
https://github.com/VCVRack/Rack.

## Reporting a vulnerability

Email **daniel.boles@gmail.com** with a description and steps to reproduce.
Please do not open a public issue for security bugs.

Expect acknowledgement within a few days. Confirmed vulnerabilities will be
fixed in the next patch release and credited in the changelog unless you prefer
otherwise.
