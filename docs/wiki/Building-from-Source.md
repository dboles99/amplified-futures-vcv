# Building from Source

How to build and install Amplified Futures locally.

---

## Requirements

| Tool | Version | Notes |
|---|---|---|
| VCV Rack SDK | 2.6.6 | Download from vcvrack.com/downloads |
| C++ compiler | GCC (MinGW64) | Via MSYS2 on Windows; system GCC on macOS/Linux |
| MSYS2 | Any recent | Windows only — provides MinGW64 toolchain |

---

## Windows (MSYS2 MinGW64)

MSYS2 uses Unix-style paths, so a Windows path like `C:\dev\Rack-SDK` is written `/c/dev/Rack-SDK` in the command below. Substitute your own two paths:

```powershell
& "C:\msys64\msys2_shell.cmd" -mingw64 -defterm -no-start -c `
  "cd /c/dev/branca-rack-modules && RACK_DIR=/c/dev/Rack-SDK make -j4"
```

Then copy the folder into your Rack plugins directory — see [[Installation]] for the path and, importantly, the name it has to have.

Set `RACK_DIR` explicitly. The Makefile default is `../..`, which is only correct if your clone sits two directories inside the SDK; it usually does not.

<details>
<summary>Author's setup (what CI mirrors)</summary>

```powershell
& "D:\dev-vcv\msys64\msys2_shell.cmd" -mingw64 -defterm -no-start -c `
  "cd /d/dev-vcv/plugins/branca-rack-modules && RACK_DIR=/d/dev-vcv/Rack-SDK make -j4"

$dst = "$env:LOCALAPPDATA\Rack2\plugins-win-x64\amplified-futures"
Remove-Item $dst -Recurse -Force -ErrorAction SilentlyContinue
Copy-Item "D:\dev-vcv\plugins\branca-rack-modules" -Destination $dst -Recurse
```

</details>

---

## macOS / Linux

```bash
export RACK_DIR=/path/to/Rack-SDK
cd path/to/branca-rack-modules
make -j4
```

Once built, see [[Installation]] for where the plugin folder goes and — the part people get wrong — what it must be named.

---

## CI (GitHub Actions)

The workflow at `.github/workflows/build.yml` builds on `windows-latest` using MSYS2 MinGW64 + Rack SDK 2.6.6. It triggers on every push to `master`.

The CI setup is the authoritative reference for build environment setup. If your local build fails, compare your environment to the CI steps.

---

## Adding a new module

New modules require 5 artifacts:
1. `src/ModuleSlug.cpp` — C++ implementation
2. `res/ModuleSlug.svg` — panel SVG
3. `plugin.json` — add module entry
4. `plugin.hpp` — declare `createModule<>` and `createModuleWidget<>`
5. `plugin.cpp` — register the module

When adding enums for params/inputs/outputs: **always append** to existing enums, never insert mid-list. VCV Rack serialises by position; inserting breaks saved patches.

---

## See also

[[Home]] · [[Design-System]] · [[Module-Reference]]
