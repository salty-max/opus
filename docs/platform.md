# platform

The engine's connection to the operating system: the video subsystem,
windows, and the event pump. Headers: `<opus/platform/platform.hpp>`,
`<opus/platform/window.hpp>`; both are reachable through `<opus/opus.hpp>`.

The platform layer is built on SDL3, which is a private dependency: no SDL
header or type appears in the public API (opus-lint enforces it).

## Ownership

```cpp
#include <opus/opus.hpp>

#include <variant>

int main() {
    auto platform = opus::Platform::create();
    if (!platform) {
        return 1;
    }
    auto window = platform->create_window({.title = "Skirmish", .size = {.width = 1280, .height = 720}});
    if (!window) {
        return 1;
    }
    bool running = true;
    while (running) {
        for (const opus::Event& event : platform->poll_events()) {
            if (std::holds_alternative<opus::QuitRequested>(event) ||
                std::holds_alternative<opus::WindowCloseRequested>(event)) {
                running = false;
            }
        }
    }
    return 0;
}
```

- **`Platform`** owns the process-wide video subsystem. There is at most one
  at a time; it is created explicitly and nothing in the engine initialises
  the operating-system layer behind the caller's back. It is used from the
  main thread only (an operating-system requirement on macOS and Windows).
- **`Window`** is created by `Platform::create_window` and owned by the
  caller. Destroying the `Window` closes the operating-system window. The
  platform must outlive its windows; a window that outlives its platform
  (whose shutdown already closed it) reports zero sizes and is safe to
  destroy.
- Both are **move-only**. A moved-from `Platform` no longer owns the video
  subsystem; a moved-from `Window` has the zero `WindowId` and reports zero
  sizes. Move assignment releases what the target owned first.

## Errors

| Function | Error | When |
|---|---|---|
| `Platform::create` | `PlatformError::AlreadyInitialized` | another `Platform` is alive |
| | `PlatformError::VideoUnavailable` | no usable video driver: no display, or the driver requested through `SDL_VIDEO_DRIVER` is missing |
| `Platform::create_window` | `WindowError::InvalidSize` | `WindowConfig::size` has a zero or negative extent |
| | `WindowError::PlatformInactive` | the `Platform` was moved from; only the platform that owns the video subsystem creates windows |
| | `WindowError::CreationFailed` | the operating system refused to create the window |

## Windows

`WindowConfig` holds the title (UTF-8), the logical size, whether the user
may resize the window, and whether it opens fullscreen on its display. Every
window is created high-DPI aware, so on high-density displays the framebuffer
has full resolution instead of being upscaled.

### Sizes

A window has two sizes and one scale, and they are not interchangeable:

| Accessor | Unit | Use it for |
|---|---|---|
| `logical_size()` | window coordinates — the unit of mouse positions | hit-testing input, window layout |
| `pixel_size()` | framebuffer pixels | render targets, viewports |
| `display_scale()` | factor | sizing UI drawn in pixels to the user's scale setting |

How they relate depends on the operating system. On macOS, window coordinates
are points: a 1280×720 window on a Retina display has a 2560×1440 pixel size
and a display scale of 2. On Windows and most Linux desktops, window
coordinates are already pixels, so the two sizes match and the user's
scaling setting (150%, 200%) appears only in `display_scale()`.

The engine's rule that holds on every system: **render in pixels, and size
UI as design units × `display_scale()`**.

## Events

`Platform::poll_events()` drains the operating system's queue and returns a
span of `opus::Event`, a `std::variant` of:

| Event | Meaning |
|---|---|
| `QuitRequested` | the user or the system asked the application to quit |
| `WindowCloseRequested` | the user asked to close `window` (close button, Alt+F4, Cmd+W) |
| `WindowResized` | `window`'s logical or pixel size changed; carries both current sizes |
| `WindowScaleChanged` | `window`'s display scale changed (moved to another display, setting changed); carries the new scale |

- The span stays valid until the next `poll_events` call.
- Size changes are **coalesced**: however many times a window's logical or
  pixel size changed since the last poll, it gets one `WindowResized` with
  the final sizes, positioned at its latest change.
- Closing is the application's decision: `WindowCloseRequested` does not
  close anything; destroying the `Window` does.
- Operating-system events with no `opus::Event` counterpart are dropped.

## Headless operation

Setting the environment variable `SDL_VIDEO_DRIVER=dummy` runs the platform
without a display: windows exist with a display scale of 1, resizes behave
normally, and nothing is shown. The platform specs use it, so CI needs no
display.

## Testing

The platform specs (`tests/platform/`) are the only specs allowed to include
SDL: they play the operating system, injecting events it would deliver and
taking the video subsystem away, through SDL itself. `tests/platform/util.hpp`
provides the headless `Platform` and readable printing of events in failed
checks.
