# Installation

Amplified Futures is a VCV Rack 2 plugin. There are three ways to get it.

---

## 1. Get the plugin

| Route | Status |
|---|---|
| **VCV Library** | Not yet listed. [library.vcvrack.com](https://library.vcvrack.com/?brand=Amplified+Futures) currently returns no results |
| **GitHub Release** | [Latest release](https://github.com/dboles99/amplified-futures-vcv/releases/latest) — **Windows, macOS and Linux**, about 18 MB each — Swarm Core ships two curated 32-sample banks |
| **Build from source** | All platforms — see [[Building-from-Source]] |

Four builds are attached to each release from 2.3.1 onward. Take the one matching your machine:

| File | For |
|---|---|
| `amplified-futures-<version>-win-x64.vcvplugin` | Windows |
| `amplified-futures-<version>-mac-arm64.vcvplugin` | macOS on Apple Silicon (M1 and later) |
| `amplified-futures-<version>-mac-x64.vcvplugin` | macOS on Intel |
| `amplified-futures-<version>-lin-x64.vcvplugin` | Linux |

Releases before 2.3.1 carry a Windows build only; on those versions macOS and Linux need the source route.

The macOS builds are not code-signed or notarised, so macOS quarantines them on download and Rack will fail to load the plugin. Clear the flag after unzipping, then start Rack:

```sh
xattr -dr com.apple.quarantine ~/Library/Application\ Support/Rack2/plugins/amplified-futures
```

---

## 2. Install it

Unzip the `.vcvplugin` into your Rack 2 user plugins directory, or copy the built plugin folder there.

| Platform | Plugin directory |
|---|---|
| Windows | `%LOCALAPPDATA%\Rack2\plugins-win-x64\` |
| macOS | `~/Library/Application Support/Rack2/plugins/` |
| Linux | `~/.Rack2/plugins/` |

> ### The folder must be named `amplified-futures`
>
> That is the plugin slug from `plugin.json`, and Rack matches the folder against it.
>
> If you built from source, the source directory is called `branca-rack-modules` — **rename it when you copy it in**. A mismatched folder name means the modules simply never appear, with no error message anywhere in the interface.

Restart Rack. The nineteen modules appear in the module browser under **Amplified Futures**.

---

## 3. Check it worked

Right-click the Rack canvas and search for `DroneCore`. If it is there, so are the other sixteen.

Swarm Core is worth a second look: it loads its sample bank on a background thread and stays silent for 2–5 seconds after you add it. That is normal. If it plays noise bursts instead of insects, the bank did not ship with your copy — check that `res/insects/insectset32/` exists inside the plugin folder.

---

## Troubleshooting

**No Amplified Futures modules in the browser.** Nearly always the folder name. Confirm the directory is literally `amplified-futures` and that `plugin.json` sits directly inside it, not one level further down.

**Rack starts but reports that a plugin failed to load.** The build does not match your Rack version. This plugin targets Rack SDK 2.6.6; check yours under Help → About.

**Patches load with missing modules after an update.** Module slugs and parameter order are stable across versions by policy, so this points to a partial install rather than a compatibility break. Delete the `amplified-futures` folder and reinstall, rather than copying over the top of it.

**Where Rack logs live.** Plugin load failures are named explicitly in the log:

| Platform | Log |
|---|---|
| Windows | `%LOCALAPPDATA%\Rack2\log.txt` |
| macOS | `~/Library/Application Support/Rack2/log.txt` |
| Linux | `~/.Rack2/log.txt` |

---

## See also

[[Building-from-Source]] · [[Home]] · [[Module-Reference]]
