# Contributing to Amplified Futures

Amplified Futures is a personal instrument series — contributions are welcome
but scope is intentionally narrow. Read this before opening a PR.

## Building locally

Requirements: Windows, MSYS2 MinGW64, Rack SDK 2.6.6.

```powershell
& "D:\dev-vcv\msys64\msys2_shell.cmd" -mingw64 -defterm -no-start -c `
  "cd /d/dev-vcv/plugins/branca-rack-modules && RACK_DIR=/d/dev-vcv/Rack-SDK make -j4"
