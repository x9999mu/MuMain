# Windows - Terminal (CMake presets)

Native MSVC build from a **Developer Command Prompt / Developer PowerShell for
VS** (so `cl`, CMake, and Ninja are on `PATH`), using the bundled presets.

See [the build guide](../README.md) for shared concepts. To cross-compile the
Windows client from Linux/WSL instead, see [wsl.md](wsl.md).

## Prerequisites

- Visual Studio C++ build tools (CMake + Ninja).
- The **.NET 10 SDK**.
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set to its
  installation directory. The presets select `x64-windows` or `x86-windows`
  and chainload the repository architecture toolchain automatically.

```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
```

## Configure and build

```powershell
# Configure (pick one)
cmake --preset windows-x64                # 64-bit
cmake --preset windows-x64-mueditor       # 64-bit + editor
cmake --preset windows-x86                # 32-bit
cmake --preset windows-x86-mueditor       # 32-bit + editor

# Build (pick the matching Debug/Release build preset)
cmake --build --preset windows-x64-mueditor-debug
cmake --build --preset windows-x64-mueditor-release
```

The configure presets are listed in `CMakePresets.json`; each has
`-debug`/`-release` build presets.

## Troubleshooting

- Run builds from an x64 Visual Studio Developer Command Prompt or Developer
  PowerShell. A regular PowerShell can let compilation start but make the
  linker select x86 Windows libraries, causing many `LNK2001`/`LNK4272`
  errors.
- Start only one CMake/Ninja build at a time. If it remains at `Re-checking
  globbed directories...` and no compiler process starts, stop the CMake and
  Ninja processes that belong to that build before retrying; concurrent
  invocations can wait on each other.
- Before testing a change, confirm the executable timestamp has been updated,
  for example `out/build/windows-x64/src/Release/Main.exe`. Compiling one
  `.cpp` file alone does not update the runnable client.

## Run

```powershell
cd out/build/windows-x64-mueditor/src/Debug
.\Main.exe
```

Run from the build output's `src/<config>` directory so the client finds its
assets, `config.ini`, and `MUnique.Client.Library.dll` (all placed there by the
post-build step).
