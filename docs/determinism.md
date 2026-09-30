# Determinism

Opus runs multiplayer games in **deterministic lockstep**: every peer
exchanges only player commands and runs the same simulation locally. Two
peers that start from the same state and apply the same commands on the
same ticks must reach bit-identical state on every tick — across
operating systems, compilers, standard libraries and CPU architectures.
Replays rely on the same property: a replay is the initial state plus the
command stream.

This document is the contract simulation code follows. The `determinism`
rule of opus-lint enforces what can be recognised syntactically; the rest
is reviewed (CLAUDE.md, self-review step 5).

## Scope

The **simulation modules** are `ecs`, `sim`, `world` and `rts`, under both
`engine/include/opus/` and `engine/src/`. Everything else — rendering,
audio, UI, platform, devtools — reads simulation state and may use floats,
clocks and threads freely. Nothing outside the simulation writes to it
except through commands.

## Rules

| Forbidden in simulation modules | Why | Use instead |
|---|---|---|
| `float`, `double`, `<cmath>`, libm calls | IEEE results differ across compilers, flags (FMA contraction, x87, vectorisation) and libm implementations | opus fixed-point scalar and vector types, LUT-based trigonometry |
| `std::unordered_map` / `unordered_set` | iteration order depends on the hash function and bucket policy, which differ between libstdc++, libc++ and MSVC | ordered containers, sorted vectors, or ECS storage with defined order |
| `std::sort`, `partial_sort`, `nth_element` | order of equal elements is unspecified and differs across standard libraries | `std::stable_sort`, or a comparator that is a total order over distinct keys |
| `<random>` engines and distributions, `rand` | distributions are implementation-defined; engines are fine but invite them | the seeded simulation RNG |
| clocks (`std::chrono`, `time`, `clock`) | wall time differs per peer | the tick counter |
| reading input, files or the network directly | peers see different things | commands scheduled for a tick |

Beyond the lint:

- **Iteration order is part of the state.** Any container the simulation
  iterates must have an order defined by simulation data alone — never by
  pointer values, allocation order, or hash values.
- **Hashed or checksummed state has no uninitialised bytes.** Padding is
  zeroed or excluded, so checksums compare state, not garbage.
- **No threads inside a tick** unless each thread's writes are disjoint
  and the merge order is fixed.
- **Integer overflow is defined behaviour or it is a bug.** Fixed-point
  arithmetic uses explicit widening and documented saturation or wrapping.

## Verification

- Every simulation module's specs include a determinism test: the same
  initial state and command stream, run twice, produce identical
  per-tick checksums.
- Scenario tests compare their per-tick checksums against **golden
  values committed to the repository**. CI runs them on GCC, Clang, Apple
  Clang and MSVC, so every toolchain is held to the same numbers — two
  toolchains that each agree with themselves but not with each other fail.
- At runtime each peer checksums its state every tick and exchanges the
  checksum; a mismatch is a desync, reported with a state dump.
