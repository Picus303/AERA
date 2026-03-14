# Windows Build And Smoke Test

This project can currently be built and smoke-tested on Windows through CMake with the MSVC toolchain discovered automatically from the local Visual Studio installation.

The current working setup on this machine is:

- CMake
- A Visual Studio installation with the C++ build tools
- 32-bit MSVC environment loaded through `vcvars32.bat`
- `NMake Makefiles` as the current stable default
- optional `Ninja`, but not yet the recommended path here

## Current Known-Good Smoke Test

The following scenario has been verified locally:

- Configure with CMake
- Build through CMake
- Run `ctest`
- Expected console output contains `hello world 1`
- Expected output file `build\\smoke\\<generator>\\bin\\decompiled_objects.txt` is generated
- Expected decompiled output contains markers such as `root`, `stdin`, `stdout`, and `pgm0`

## Why This Is Documented

This note exists so we do not have to rediscover the same build quirks while we prepare the larger cleanup and toolchain modernization.

It is intentionally tactical, not aspirational.

## Recommended Command

Use the smoke-test script in [tests/smoke_hello_world.ps1](C:/Users/pille/Documents/GitHub/AERA/tests/smoke_hello_world.ps1).

That script:

- locates the installed MSVC toolchain with `vswhere`
- loads the Visual Studio 32-bit build environment
- configures CMake with `NMake Makefiles` by default
- accepts `-Generator ninja` for explicit experiments with `Ninja`
- builds the project through CMake
- runs `ctest` and executes the smoke test

If the script fails early on a clean Windows machine, the likely missing tools are:

- `cmake`
- Visual Studio Build Tools or Visual Studio Community with the C++ workload
- optionally `ninja`

## Current Limitations

- `Ninja` is visible on this machine, but it currently stalls during the MSVC compiler ABI/link detection step in CMake
- This is still a Windows-first build
- The script is Windows-specific
- The code still emits many warnings during build
- TCP/protobuf support is disabled by default in the new CMake flow

That is acceptable for now because the goal is to establish a regression baseline before the large cleanup.
