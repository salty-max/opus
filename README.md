# Opus

A reusable 2D game engine in C++23, built RTS-first: deterministic lockstep
simulation, SDL3 + SDL_GPU rendering (Vulkan / Metal / D3D12), Lua
scripting, for macOS, Windows and Linux.

```bash
lefthook install                        # once per clone
cmake --workflow --preset quick         # build + test
cmake --build --preset debug --target run
```

- [PLAN.md](PLAN.md) — roadmap
- [docs/development.md](docs/development.md) — toolchain, gates, conventions, releases
- [docs/determinism.md](docs/determinism.md) — the simulation contract
- [CHANGELOG.md](CHANGELOG.md)
