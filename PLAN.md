# Opus — 2D RTS-oriented Game Engine (C++23)

## Goals

- **Reusable engine**: a library that games link against, with a clean public API. It is not tied to one game.
- **First target genre: RTS.** That covers tilemaps, many entities, selection, pathfinding, fog of war and UI, so it also covers most other 2D genres.
- **Cross-platform**: macOS, Windows and Linux from day one.
- **Multiplayer-ready**: the simulation is deterministic and runs in lockstep.
- **Scale**: about 500 active units at 60 fps render and a fixed simulation tick.

## Key decisions

| Area | Decision | Why |
|---|---|---|
| Language | C++23, CMake + Ninja, presets | Modern features (`std::expected`, `std::print`, ranges), same build on all OSes |
| Platform | **SDL3** | Window, input, audio, file I/O, threads on all 3 OSes |
| GPU | **SDL_GPU** | Vulkan-style API → Vulkan (Win/Linux), Metal (macOS), D3D12 (Win) |
| Shaders | HLSL → SPIR-V / MSL / DXIL via **SDL_shadercross**, compiled at build time | One shader source for all backends |
| Entities | **Own ECS** (sparse-set, generational IDs) | Full control over determinism, iteration order, snapshots |
| Simulation | **Deterministic lockstep**, fixed tick (e.g. 20 Hz), **fixed-point math**, rendering interpolated | Required for RTS multiplayer; replays come almost free |
| Scripting | **Lua 5.4 + sol2** | Unit definitions, behaviors, map scripts, modding |
| Camera / view | Projection abstracted (orthographic top-down **and** isometric) | Art style is still undecided |
| Tooling | **Dear ImGui** overlays, **map editor**, **hot reload** | |
| Dependencies | **CMake FetchContent**, pinned tags | No extra tool, identical on all OSes |
| Errors | **No exceptions, no RTTI**; `std::expected<T, ModuleError>` | Explicit, checkable error paths |
| Process | Gero-style: specs first, issue → branch → PR, self-review loop, changesets | See `CLAUDE.md`, `docs/development.md` |
| Tests | doctest, mirrored spec per engine file, every build mode + ASan/UBSan, determinism tests against golden checksums | |

### Determinism rules (non-negotiable once M5 lands)
- Simulation code uses **no floats**. It uses a fixed-point type (Q32.32 in `int64_t` or Q16.16) and LUT-based trig/sqrt.
- The simulation uses one **seeded deterministic RNG** (e.g. PCG). It never calls `rand()` or reads time.
- ECS iteration order is deterministic. The simulation never iterates an unordered container.
- Every player action becomes a **command** scheduled for tick N. The simulation never reads input directly.
- Lua code that affects the simulation only sees fixed-point userdata. Lua used for UI and rendering may use floats.
- No `-ffast-math`. At the end of each tick the simulation computes a checksum of its state, which is used for desync detection.

## Architecture

```
opus/
├── engine/                 # libopus (static lib) — the reusable engine
│   ├── include/opus/...    # public API
│   └── src/
│       ├── core/           # log, assert, memory/allocators, containers, time, fixed-point, RNG
│       ├── platform/       # SDL3 wrapper: window, input, filesystem, threads
│       ├── render/         # SDL_GPU device, sprite batcher, atlas, camera, text, debug draw
│       ├── ecs/            # entity registry, sparse sets, queries, system scheduler, snapshots
│       ├── sim/            # fixed-tick loop, command queue, checksums
│       ├── world/          # tilemap, map format, spatial hash, fog of war
│       ├── rts/            # selection, orders, pathfinding, steering, combat, economy, production
│       ├── script/         # Lua VM, bindings, sandbox, hot reload
│       ├── audio/          # SDL3 audio mixing, positional sound
│       ├── net/            # lockstep protocol, transport, replay record/playback
│       ├── ui/             # in-game UI (HUD, minimap, command card)
│       └── devtools/       # ImGui integration, inspectors, map editor
├── shaders/                # HLSL sources
├── sandbox/                # sample RTS game using the engine
├── editor/                 # standalone map editor executable (or a mode of sandbox)
├── tests/
└── cmake/                  # dependency + shader compile helpers
```

Frame loop:
```
poll input → translate to commands → (net) exchange commands
while accumulator >= tick: sim.step(commands for tick N) → checksum
render(interpolate(prev_state, cur_state, alpha)) → ImGui → present
```

## Milestones

**M0 — Toolchain** ✅
Xcode Clang 17, CMake, Ninja, Homebrew LLVM 23 (clang-tidy, clang-format), Doxygen, lefthook, convco, actionlint.

**M1 — Project skeleton & process** ✅
- `engine` lib + `sandbox` + `tests`, doctest via FetchContent, install/CPack packaging.
- Gates: `quick` / `verify` / `ci`, presets for Debug, RelWithDebInfo, Release, MinSizeRel, ASan/UBSan.
- `opus-lint` (project rules, determinism rule), strict clang-tidy, Doxygen gate, doc-example compile gate, clang-format.
- Hooks, commit convention, changesets, CI / changeset-check / release workflows (GitHub repo pending).
- First module: `core/log` (`Logger`, `LogLevel`, `std::expected` parsing).

**M2 — Platform & main loop**
- SDL3 via FetchContent; window, resize and high-DPI handling.
- Input: keyboard, mouse, edge-scroll, and an action-mapping layer.
- Fixed-tick sim loop + variable render with interpolation, and a frame timer.

**M3 — Renderer**
- SDL_GPU device, shader build pipeline (shadercross).
- Batched sprite renderer (instanced quads), texture atlas packing, sprite animation.
- Camera with ortho/iso projection, zoom and pan; screen↔world picking.
- Text rendering (stb_truetype first, MSDF later), debug-draw lines, rects and circles.
- **Bench gate** (`bench-check`, as in Gero): benchmark harness, committed baselines and a CI gate, landing with sprite batching as the first performance-critical module. Deferred from M1 by maintainer decision: a gate with nothing real to guard is premature.

**M4 — ECS**
- Generational entity IDs; sparse-set component pools; multi-component views.
- Deterministic iteration; system scheduler with ordered phases.
- Snapshot/serialize whole world; world checksum.
- ECS iteration and query benchmarks join the bench gate, budgeted against the ~500-unit target.

**M5 — Deterministic core**
- Fixed-point type + vec2, LUT trig/sqrt/atan2, PCG RNG, command queue keyed by tick.
- Determinism test harness: the same inputs must give the same checksum, on every platform in CI.

**M6 — World**
- Grid/tilemap with layers, passability, map file format (JSON/binary).
- Spatial hash for proximity queries; fog of war (per-player visibility grid); minimap.

**M7 — RTS systems**
- Box/click selection, control groups; orders: move, attack, attack-move, stop, gather, build.
- Pathfinding: A* on grid (+ path smoothing), with a path request budget per tick.
- Steering: separation and arrival, simple formations; collision between units.
- Combat (range, cooldown, projectiles), health, death.
- Economy (resources, gatherers), production queues, building placement & construction.

**M8 — Lua scripting**
- Lua 5.4 + sol2; unit and building definitions in Lua tables.
- Behavior hooks (on_spawn, on_tick, on_death), map/trigger scripts, sandboxed environment.

**M9 — Tools**
- Dear ImGui (SDL3 + SDL_GPU backends): entity inspector, system timings, perf graphs, cheats and toggles.
- Hot reload of Lua, textures and data files (file watcher).
- **Map editor**: paint tiles and passability, place units, resources and start positions; save and load.

**M10 — Audio**
SDL3 audio (+ SDL_mixer or miniaudio), sound categories, positional attenuation relative to the camera.

**M11 — Networking**
- Lockstep: turn-based command exchange with input delay; transport via ENet (UDP).
- Lobby / host-join; desync detection via per-tick checksums plus state dump.
- Replay record & playback (same code path as networking).

**M12 — Sample game & packaging**
- A small playable RTS in `sandbox/`: 2 factions, ~5 unit types, 1 resource, skirmish vs a basic AI.
- Asset packing; app bundles and installers per OS.

## Dependencies (all via FetchContent, pinned)
SDL3 · SDL_shadercross · Dear ImGui · Lua 5.4 · sol2 · stb (image, truetype, rect_pack) · nlohmann/json or glaze · ENet · doctest

## Open questions (to decide later)
- Top-down vs isometric art. The camera supports both, and the choice affects tile art and depth sorting.
- Q16.16 vs Q32.32 fixed-point. This depends on map size and needed precision; decide in M5.
- Game AI: behavior trees vs utility AI for the computer opponent (M12).
- Whether the map editor is a separate executable or an in-game mode.
