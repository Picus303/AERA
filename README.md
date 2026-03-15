# AERA

This repository is being reset around a smaller baseline.

What remains active today:
- `core/`: foundational runtime utilities formerly carried through a git submodule
- `r_code/`: low-level in-memory representation
- `r_comp/`: Replicode compilation and decompilation
- `r_exec/`: execution and reasoning runtime
- `AERA/`: application entry point and baseline settings
- `modules/`: built-in extension modules registered in-process
- `examples/`: Replicode assets kept outside the app/runtime code
- `tests/`: smoke-test entry points

Inside `r_exec/`, the active split is now responsibility-driven:
- `attention/`
- `bootstrap/`
- `construction/`
- `controllers/`
- `diagnostics/`
- `evaluation/`
- `extensions/`
- `learning/`
- `metadata/`
- `monitoring/`
- `overlays/`
- `runtime/`
- `scheduling/`

What has been deliberately removed from the active baseline:
- Visual Studio solutions and project files
- legacy docs and changelogs
- deprecated Replicode examples and demos
- Webots assets and experiment-specific folders
- optional TCP/protobuf and output window build paths

The current supported build path is Windows-first through CMake and the smoke test:

```powershell
powershell -ExecutionPolicy Bypass -File tests\smoke_hello_world.ps1
```

The current supported runtime path is `test_mem`. The smoke test is the regression baseline while the repository is being restructured.

See [overview](C:/Users/pille/Documents/GitHub/AERA/docs/overview.md) and [recovery-roadmap](C:/Users/pille/Documents/GitHub/AERA/docs/recovery-roadmap.md).
