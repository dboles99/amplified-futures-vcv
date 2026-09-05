# Contributing to Amplified Futures

Amplified Futures is a personal instrument series — contributions are welcome
but the scope is intentionally narrow. Read this before opening a PR.

## Building locally

Requirements: the [Rack SDK](https://vcvrack.com/downloads) 2.6.6, and a C++11
toolchain — MSYS2 MinGW64 on Windows, clang or gcc elsewhere.

```sh
RACK_DIR=/path/to/Rack-SDK make -j4
```

On Windows, run that inside an MSYS2 MinGW64 shell:

```powershell
& "C:\msys64\msys2_shell.cmd" -mingw64 -defterm -no-start -c `
  "cd /path/to/branca-rack-modules && RACK_DIR=/path/to/Rack-SDK make -j4"
```

`make dist` builds the installable `.vcvplugin` package.

To install for testing, copy the plugin directory to Rack's plugin folder. **It
must be named `amplified-futures`** — the directory name has to match the slug
in `plugin.json`, or Rack loads nothing and reports no error.

## Tests

The DSP cores in `src/dsp/` have no Rack dependency, so they are tested
offline. There are nine suites:

```sh
cd tests && make test
```

Everything must pass before a PR.

## What a PR needs

- **Tests first for anything behavioural.** New DSP goes in `src/dsp/` as a
  header with no Rack dependency, plus a suite in `tests/`.
- **Never insert into an enum.** Rack serialises parameters, inputs, outputs and
  lights by position — appending is safe, inserting or reordering silently
  repoints every saved patch. `python scripts/check-patch-compat.py` checks this
  against the last release.
- **Panels have rules.** No `<text>` elements (nanosvg will not render them with
  external fonts — convert to paths in Inkscape), labels above widgets rather
  than at widget-centre y, every port labelled, and `1 HP = 15 px` exactly.
- **Update the docs with the code.** `docs/wiki/` is the manual and
  `docs/modules/` the parameter reference; both are checked by
  `python tools/check_wiki.py`.
- **Add a CHANGELOG entry.** Bumping the version without one is how two
  different builds ended up both claiming to be 2.2.0.

## Scope

Bug fixes, performance work, portability fixes and documentation are all
welcome. New modules are usually not — the series is a designed set rather than
a collection. Open an issue before building one.

## Licence

Contributions are accepted under the MIT licence, matching the rest of the
plugin. The audio under `res/insects/` is CC BY 4.0 and is not yours or mine to
relicense — see `res/insects/banks/ATTRIBUTION.md`.
