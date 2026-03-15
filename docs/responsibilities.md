# Responsibilities

This pass treats separation of responsibilities as a hard architectural rule rather than a cosmetic cleanup step.

## Principles

- `core/` owns shared infrastructure only.
- `r_code/` owns low-level runtime data structures and serialization.
- `r_comp/` owns Replicode parsing, preprocessing, metadata, and decompilation.
- `r_exec/` owns runtime behavior only: execution, scheduling, controllers, monitoring, learning, diagnostics, and extension registration.
- `AERA/` owns application bootstrap, runtime configuration, and the baseline IO device used by the smoke test.
- `modules/` owns built-in operator/program/callback modules that plug into `r_exec` through explicit registrars.
- `examples/` owns sample Replicode assets and should stay outside the runtime code folders.

## Active Structure

- `AERA/bootstrap/`: executable entry point, runtime orchestration, export helpers.
- `AERA/config/`: runtime settings model and XML loading.
- `AERA/io/`: baseline `test_mem` IO device.
- `r_exec/attention/`: auto-focus and relevance filtering.
- `r_exec/bootstrap/`: public runtime bootstrap API and initialization pipeline.
- `r_exec/construction/`: constructors and factories for markers, facts, simulations, and related runtime objects.
- `r_exec/diagnostics/`: asynchronous decompilation and runtime trace sinks.
- `r_exec/evaluation/`: operator contexts, binding maps, and expression evaluation support.
- `r_exec/extensions/`: extension registries and registration contracts.
- `r_exec/metadata/`: opcode tables and runtime-wide symbolic identifiers.
- `r_exec/runtime/`: long-lived runtime state such as memory, groups, views, object wrappers, and the model base.
- `r_exec/scheduling/`: reduction and time scheduling primitives.
- `modules/builtin/`: built-in operators, callbacks, and native programs registered in-process.

## Next Refactors

1. Reduce transitive includes so headers expose only the types they really need.
2. Add more characterization tests around compile/decompile and runtime output.
3. Clean up historical comments and compiler warnings that now stand out clearly.
4. Keep pushing platform-specific code toward explicit edge modules.
