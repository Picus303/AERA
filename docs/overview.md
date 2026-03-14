# Overview

This repository is being narrowed to a baseline that is small enough to understand, build, and test reliably.

The active architecture is:

- `core/`: shared utility layer used by the rest of the codebase
- `r_code/`: object model and low-level runtime representation
- `r_comp/`: compiler and decompiler for Replicode sources
- `r_exec/`: execution engine, memory, scheduling, monitoring, and learning logic
- `AERA/`: executable bootstrap and runtime settings
- `usr_operators/`: dynamically loaded operators used by the runtime

The active execution flow is:

1. Load `AERA/settings.xml`
2. Load `usr_operators.dll`
3. Compile `user.classes.replicode` and `main.replicode`
4. Create `test_mem`
5. Load seed objects and run the runtime
6. Export decompiled objects for inspection

The current baseline intentionally excludes historical integrations that are not needed to preserve this path.
