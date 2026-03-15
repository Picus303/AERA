# Overview

This repository is being narrowed to a baseline that is small enough to understand, build, and test reliably.

The active architecture is:

- `core/`: shared utility layer used by the rest of the codebase
- `r_code/`: object model and low-level runtime representation
- `r_comp/`: compiler and decompiler for Replicode sources
- `r_exec/`: execution engine, memory, scheduling, monitoring, learning, diagnostics, and extension registries
- `AERA/`: executable bootstrap, runtime configuration, and baseline IO device
- `modules/`: in-process extension modules registered during runtime bootstrap
- `examples/`: Replicode scenarios used by the baseline

The current responsibility split inside the app/runtime boundary is:

- `AERA/bootstrap/`: process entry point and orchestration
- `AERA/config/`: XML-backed runtime settings
- `AERA/io/`: baseline `test_mem` implementation
- `r_exec/attention/`: auto-focus and relevance selection logic
- `r_exec/bootstrap/`: runtime bootstrap API
- `r_exec/construction/`: factory types for runtime objects and markers
- `r_exec/diagnostics/`: decompiler threads and debug sinks
- `r_exec/evaluation/`: contexts, binding maps, and operators
- `r_exec/extensions/`: callback/program/operator registration support
- `r_exec/metadata/`: opcode registry and runtime metadata bindings
- `r_exec/runtime/`: memory, groups, views, model base, and runtime object wrappers
- `r_exec/scheduling/`: reduction/time cores and jobs
- `modules/builtin/`: built-in operators, callbacks, and C++ programs registered in-process

The active execution flow is:

1. Load `AERA/settings.xml`
2. Compile `user.classes.replicode` and initialize built-in extensions
3. Compile `main.replicode`
4. Create `test_mem`
5. Load seed objects and run the runtime
6. Export decompiled objects for inspection

The current baseline intentionally excludes historical integrations that are not needed to preserve this path.
