# Recovery Roadmap

This repository is now treated as a recovery and reconstruction effort rather than a legacy preservation effort.

Current completed steps:

- migrated the Windows build to CMake
- added a smoke test through `ctest`
- removed Visual Studio project files
- removed legacy docs, demos, and old example sets
- integrated `CoreLibrary` into the main repository
- reduced the runtime baseline to `test_mem`
- started separating runtime responsibilities into bootstrap, diagnostics, extensions, config, and IO layers

Recommended next steps:

1. Finish shrinking `test_mem` by separating its simulated environments into smaller units.
2. Add 2-4 more characterization tests around compile/decompile and runtime output.
3. Decide whether `usr_operators/` stays as a dynamic plugin boundary or moves in-process.
4. Reduce historical headers and comments that still describe removed features.
5. Revisit generator support and fix the `Ninja + MSVC` configuration path separately from runtime cleanup.
