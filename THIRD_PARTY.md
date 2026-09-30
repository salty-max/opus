# Third-party software

Every dependency is declared in [`cmake/OpusDependencies.cmake`](cmake/OpusDependencies.cmake),
pinned to an exact release, and listed here in the same PR that adds it.

## Shipped with the engine

Linked into `libopus` and therefore into every game built with it. Their
license files are installed under `share/licenses/opus/<name>/` in every
package.

| Dependency | Version | License | Source |
|---|---|---|---|
| SDL3 | 3.4.16 | zlib | https://github.com/libsdl-org/SDL/tree/release-3.4.16 |

## Development only

Used to build and test Opus itself; never linked into the engine or shipped.

| Dependency | Version | License | Source |
|---|---|---|---|
| doctest | 2.5.3 | MIT | https://github.com/doctest/doctest/tree/v2.5.3 |
