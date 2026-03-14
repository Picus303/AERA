# Responsibilities

This pass treats separation of responsibilities as a hard architectural rule rather than a cosmetic cleanup step.

## Principles

- `core/` owns shared infrastructure only.
- `r_code/` owns low-level runtime data structures and serialization.
- `r_comp/` owns Replicode parsing, preprocessing, metadata, and decompilation.
- `r_exec/` owns runtime behavior only: execution, scheduling, controllers, monitoring, learning, diagnostics, and extension loading.
- `AERA/` owns application bootstrap, runtime configuration, and the baseline IO device used by the smoke test.
- `examples/` owns sample Replicode assets and should stay outside the runtime code folders.

## Active Structure

- `AERA/bootstrap/`: executable entry point, runtime orchestration, export helpers.
- `AERA/config/`: runtime settings model and XML loading.
- `AERA/io/`: baseline `test_mem` IO device.
- `r_exec/bootstrap/`: public runtime bootstrap API and initialization pipeline.
- `r_exec/diagnostics/`: asynchronous decompilation and runtime trace sinks.
- `r_exec/extensions/`: extension registries and shared-library bridge types.

## Next Refactors

1. Keep shrinking `test_mem` by separating continuous motion, cart-pole, and discrete motion concerns.
2. Split the remaining large `r_exec` units by capability instead of historical naming.
3. Reduce transitive includes so headers expose only the types they really need.
4. Move sample Replicode assets out of `AERA/` when the runtime baseline no longer depends on their current location.
