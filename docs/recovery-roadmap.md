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
- added characterization tests for runtime settings and moved evaluation code into its own runtime subdomain
- completed the responsibility-driven split of the remaining runtime core into attention, construction, metadata, runtime, and scheduling layers
- replaced the old `usr_operators` DLL boundary with an in-process builtin module registration path

Recommended next steps:

1. Add 1-2 more characterization tests around compile/decompile and runtime output.
2. Reduce historical headers and comments that still describe removed features.
3. Triage and eliminate the remaining compiler warnings that now stand out more clearly.
4. Revisit generator support and fix the `Ninja + MSVC` configuration path separately from runtime cleanup.
