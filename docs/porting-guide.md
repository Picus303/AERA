# Porting Guide

This document is a language-agnostic guide for reimplementing the current AERA baseline in another language or runtime.

It is not a "rewrite everything" manifesto. Its goal is to help a porter preserve the behavior that matters, choose a sane order of attack, and avoid carrying over accidental C++ design constraints.

## Scope

This guide targets the current active baseline only:

- `core/`
- `r_code/`
- `r_comp/`
- `r_exec/`
- `modules/builtin/`
- `AERA/`
- `examples/replicode/`
- `tests/`

It does not assume that removed integrations, demos, or historical subsystems will be ported.

## Baseline To Preserve

Today, the smallest meaningful end-to-end baseline is:

1. Load runtime settings from `AERA/settings.xml`.
2. Compile `examples/replicode/user.classes.replicode`.
3. Initialize built-in operators, callbacks, and native programs.
4. Compile `examples/replicode/main.replicode`.
5. Create the `test_mem` host runtime.
6. Load the seed objects into memory.
7. Run in diagnostic time.
8. Export decompiled objects and verify the expected markers.

The current executable path is orchestrated in `AERA/bootstrap/runtime_runner.cpp`.

The current regression entry point is:

```powershell
powershell -ExecutionPolicy Bypass -File tests\smoke_hello_world.ps1
```

Any port should preserve this baseline before it attempts broader feature coverage.

## What Must Survive A Port

The port does not need to preserve file names, class layouts, or memory tricks. It does need to preserve:

- the Replicode compilation pipeline
- the metadata and opcode model
- the object/view/group runtime concepts
- the scheduling split between reduction work and time work
- diagnostic time execution
- extension registration semantics
- the observable behavior covered by the smoke and characterization tests

In other words: preserve contracts and behavior, not the current C++ shape.

## Architecture Summary

The current codebase can be understood as six layers.

### 1. Foundation

`core/` provides platform utilities, threading primitives, clocks, XML parsing, and low-level helpers.

For a port, treat this as replaceable infrastructure rather than domain logic.

### 2. Runtime IR And Serialization

`r_code/` is the low-level representation of objects, views, atoms, images, and memory interfaces.

This is the closest thing to a runtime IR. It is important because:

- it defines how compiled Replicode is represented in memory
- it drives image serialization and deserialization
- it is the boundary between compilation and execution

### 3. Compiler And Metadata

`r_comp/` owns:

- preprocessing
- parsing
- class metadata
- decompilation
- the higher-level `Image` wrapper used by the runtime bootstrap

The most important outputs of this layer are `Metadata` and `Image`.

### 4. Runtime

`r_exec/` owns:

- bootstrap and opcode initialization
- runtime memory (`_Mem`)
- object wrappers and views
- groups and overlays
- evaluation contexts and operators
- controllers
- monitoring
- learning and pattern extraction
- scheduling

This is the cognitive and temporal core of the system.

### 5. Built-In Extensions

`modules/builtin/` registers built-in:

- operator overloads
- callbacks
- native C++ programs

This is now an in-process module, not a dynamically loaded plugin.

### 6. Host Application

`AERA/` is a host layer that:

- loads config
- initializes the runtime
- instantiates `test_mem`
- runs the scenario
- exports artifacts

This host layer should remain thin in any port.

## Concepts Worth Preserving Explicitly

These concepts should probably exist in some recognizable form after a port:

- `Atom`: the smallest typed value unit
- `Code`: runtime object payload plus references
- `View`: projection of an object into a group with timing/saliency metadata
- `Group`: container and propagation space for views
- `Metadata`: compiled class/operator/function registry
- `Image`: serialized or serializable object graph
- `Seed`: compiled base image loaded before the main source
- `_Mem`: orchestrator for runtime execution
- `Controller`: behavior attached to runtime structures
- `Overlay`: intermediate runtime state for evaluation and matching

Names can change, but the distinctions should remain.

## Porting Principles

### Preserve Behavior, Not Representation

Do not start by cloning C++ classes one-for-one.

Prefer:

- a fresh module map
- explicit ownership and lifetimes
- immutable metadata when possible
- well-defined runtime registries
- testable boundaries between compiler, runtime, and host

### Port The Sequential Story First

The most important implementation order is:

1. config
2. compilation
3. image/metadata loading
4. runtime in diagnostic time
5. decompilation/export
6. built-in extensions
7. only then real concurrency and timing fidelity

Diagnostic time is the best first target because it removes most concurrency noise while preserving the runtime model.

### Use Tests As The Executable Spec

The current tests are small, but they already define important behavior:

- settings parsing
- built-in module registration
- smoke run behavior

Before a serious port begins, expand this characterization layer further in the current codebase or reproduce equivalent tests in the target implementation.

### Keep Platform Code At The Edge

The port should isolate:

- clocks
- threading primitives
- file I/O
- XML/config loading
- console/log sinks

These should not bleed into the domain model.

## Recommended Target Shape

A language-independent target layout could look like this:

```text
foundation/
config/
ir/
compiler/
runtime/
extensions/
host/
examples/
tests/
```

With responsibilities:

- `foundation/`: platform services and generic utilities
- `config/`: runtime settings model and loading
- `ir/`: atoms, objects, views, image serialization
- `compiler/`: preprocessing, parsing, metadata, decompilation
- `runtime/`: memory, groups, evaluation, scheduling, controllers, monitoring, learning
- `extensions/`: built-in operator/program/callback registration
- `host/`: app bootstrap and `test_mem`-equivalent integration
- `tests/`: characterization and regression suites

## Suggested Porting Order

### Phase 1: Freeze The Spec

Before porting:

- keep the current smoke test green
- capture the decompiled output and console markers
- add more characterization tests if necessary
- document which inputs are considered canonical

Canonical inputs today are:

- `AERA/settings.xml`
- `examples/replicode/user.classes.replicode`
- `examples/replicode/main.replicode`

### Phase 2: Port The Stable Data Model

Port first:

- atoms
- object/image structures
- metadata containers
- object-name mapping
- opcode-name mapping

This is the best place to define new ownership rules and safer serialization APIs.

### Phase 3: Port The Compiler Boundary

The next stable milestone is:

- load settings
- compile the seed classes
- compile the main source
- produce metadata and image artifacts

At this stage, no full runtime execution is required yet.

### Phase 4: Port Built-In Extension Registration

Recreate the current contract represented by:

- `r_exec/extensions/extension_registration.h`
- `modules/builtin/module.h`

This should remain a simple explicit registry API.

Avoid reintroducing dynamic plugin loading unless it is a deliberate product requirement later.

### Phase 5: Port Runtime In Diagnostic Time

This is the first meaningful runtime milestone:

- seed loading
- object graph instantiation
- builtin registration
- sequential job execution
- deterministic diagnostic time stepping
- final export/decompilation

The runtime does not need to be multithreaded at this point to be valuable.

### Phase 6: Port Full Scheduling And Concurrency

Only after diagnostic time passes should you introduce:

- reduction workers
- time workers
- queues
- synchronization
- real-time execution

This keeps behavioral debugging tractable.

## Hard Parts And Why They Matter

### 1. Object Graph Identity

The current runtime mixes:

- object code
- object references
- OIDs
- views held in groups
- back-references through runtime structures

A port should decide early:

- which objects are identity-bearing
- which structures are immutable
- how references are represented
- where OIDs are assigned and stored

This is a core architectural decision, not a syntax translation detail.

### 2. Metadata And Opcode Stability

The runtime depends heavily on opcode lookups and metadata-derived identity.

You should preserve:

- deterministic opcode initialization
- stable mapping from names to opcodes
- stable mapping from opcodes back to semantic meaning

If the target implementation makes these mappings implicit or unstable, subtle behavioral drift is likely.

### 3. Images And Serialization

The current code distinguishes between:

- low-level image storage in `r_code`
- higher-level image handling in `r_comp`

A port may simplify this, but it should still preserve the separation between:

- serialized representation
- mutable runtime object graph

### 4. Time Semantics

The runtime uses two time stories:

- real time
- diagnostic time

Diagnostic time is not just a test convenience. It is a very useful semantic mode for deterministic replay and debugging.

A port should preserve it explicitly.

### 5. Runtime Scheduling

`_Mem` coordinates two families of work:

- reduction jobs
- time jobs

Even if a new implementation changes the queue or worker model, that semantic distinction is worth preserving until there is strong evidence it can be collapsed safely.

### 6. Built-In Extensions

The new extension model is intentionally simple:

- built-in modules receive opcode lookup capability
- they register operators, programs, and callbacks explicitly

This contract should survive a port because it creates a clean seam between runtime and host-specific/native behavior.

### 7. Learning And Monitoring

The most coupled area in the codebase is still inside `r_exec/`.

If time is limited, do not start by porting all of learning and monitoring in one shot. Instead:

- port enough runtime behavior to keep the smoke path alive
- port monitoring hooks next
- port learning/pattern extraction after the core runtime is stable

## What Not To Port Immediately

Do not make these the first milestone:

- full multithreading fidelity
- every historical example
- every optimization or memory trick
- every old debug pathway
- platform-specific convenience behavior

A port that cleanly reproduces the baseline in diagnostic time is more valuable than a partial port that attempts everything at once.

## Suggested Validation Strategy

### Minimum Validation

Any new implementation should, at minimum:

- load settings successfully
- compile the seed classes successfully
- register built-in extensions successfully
- compile and run the smoke scenario successfully
- export decompiled objects containing the expected markers

### Better Validation

As the port grows, compare:

- console markers
- exported object counts
- decompiled object names
- model export markers
- builtin registration coverage

### Best Validation

Create cross-implementation golden tests where:

- the current C++ baseline produces reference outputs
- the target implementation reproduces those outputs
- differences are reviewed intentionally rather than discovered ad hoc

## A Good First Port Milestone

A very good first milestone is:

- one process
- one settings file
- one builtin module
- one host IO device
- diagnostic time only
- smoke test green

That milestone is small enough to finish and large enough to prove the architecture.

## A Practical Definition Of "Port Complete"

For the current baseline, "port complete" should probably mean:

- the smoke scenario passes
- the builtin registration contract is implemented
- settings and seed compilation work
- diagnostic time works
- decompiled objects can be exported
- the main runtime concepts are represented cleanly

It does not need to mean total parity with every historical subsystem.

## Migration Strategy Options

### Option A: Spec-First Rewrite

Keep improving characterization in the current codebase, then reimplement from the tests.

Best when:

- the target language is very different
- you want a cleaner architecture

### Option B: Shadow Port

Run both implementations side by side and compare artifacts incrementally.

Best when:

- artifact comparison is feasible
- you want lower behavioral risk

### Option C: Hybrid

Port stable layers first as a rewrite, then shadow-compare the runtime layers.

This is probably the best fit for the current repository.

## Recommended Decision Rules During A Port

When choosing whether to copy or redesign a concept:

- preserve semantics if tests or runtime behavior depend on it
- redesign representation if the current C++ shape exists mainly for memory management or historical build constraints
- delay optimization until the diagnostic-time baseline is green
- create explicit interfaces where the current code relies on incidental include or pointer coupling

## Current Files That Matter Most To A Porter

If someone needs a concrete reading order before porting, start here:

1. `docs/overview.md`
2. `docs/responsibilities.md`
3. `AERA/bootstrap/runtime_runner.cpp`
4. `r_exec/bootstrap/init.h`
5. `r_comp/model/segments.h`
6. `r_code/object.h`
7. `r_exec/runtime/mem.h`
8. `r_exec/extensions/extension_registration.h`
9. `modules/builtin/module.h`
10. `tests/smoke_hello_world.ps1`

## Final Recommendation

Treat the port as a product reconstruction effort, not a syntax conversion effort.

The right order is:

1. preserve the baseline
2. strengthen the characterization tests
3. port the stable data and compiler boundaries
4. port the runtime in diagnostic time
5. add concurrency and broader feature coverage later

If this order is respected, the port can become simpler and more maintainable than the current implementation without losing the behavior that makes the project worth keeping.
