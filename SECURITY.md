# Security Policy

## Supported versions

| Version | Supported |
|---|---|
| 2.3.x | Yes |
| 2.2.x | Security fixes only |
| < 2.2 | No |

## Reporting a vulnerability

Open a [private security advisory](https://github.com/dboles99/amplified-futures-vcv/security/advisories/new),
or email daniel.boles@gmail.com. Please do not open a public issue for a
vulnerability. Expect an acknowledgement within a week.

## What this plugin does

It is a VCV Rack 2 plugin: a shared library Rack loads into its own process. It
has the same access to your machine that Rack does.

- **No network access.** No sockets, no HTTP, no telemetry, no update checks.
- **No process execution.** No `system`, `popen`, or subprocess calls.
- **No environment inspection.** No `getenv`.
- **No writes.** The plugin writes no files anywhere.
- **Reads, in one place only.** Swarm Core reads `.wav` files from its own
  `res/insects/banks/` directory inside the plugin folder, on a background
  thread started when the module is created. That is the plugin's entire file
  I/O surface — see `src/dsp/WavRead.hpp`.

Patch state is handled by Rack through `dataToJson`/`dataFromJson`; the plugin
never touches the patch file itself.

## The WAV reader

Swarm Core parses WAV files, which is the only place this plugin consumes
data it did not create. As of 2.3.0 the parser rejects malformed input rather
than trusting it: zero-channel files (which previously caused a divide by zero),
unsupported bit depths, truncated data chunks and non-RIFF files all fail
cleanly. It is covered by 26 assertions in `tests/test_wav_read.cpp`.

If you point it at your own audio and find input that crashes it, that is a
security report and the address above is the right place for it.
