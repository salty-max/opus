# platform

The engine's connection to the operating system: the video subsystem,
windows, the event pump, and input. Headers: `<opus/platform/platform.hpp>`,
`<opus/platform/window.hpp>`, `<opus/platform/input.hpp>`,
`<opus/platform/actions.hpp>`; all are reachable through `<opus/opus.hpp>`.

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
| `KeyPressed` / `KeyReleased` | a physical key went down / up; auto-repeat is not reported |
| `MouseButtonPressed` / `MouseButtonReleased` | a mouse button went down / up; carries the `Cursor` where it happened |
| `MouseMoved` | the cursor moved within a window; carries the new `Cursor` |
| `MouseWheel` | the wheel or trackpad scrolled; positive `delta.y` scrolls away from the user whatever the system's "natural scrolling" setting |
| `CursorEntered` / `CursorLeft` | the cursor entered / left a window |

- The span stays valid until the next `poll_events` call.
- Size changes are **coalesced**: however many times a window's logical or
  pixel size changed since the last poll, it gets one `WindowResized` with
  the final sizes, positioned at its latest change.
- Closing is the application's decision: `WindowCloseRequested` does not
  close anything; destroying the `Window` does.
- Operating-system events with no `opus::Event` counterpart are dropped,
  and so are keys with no `Key` and mouse buttons beyond the five
  `MouseButton` names.
- A `Cursor` gives a position in both coordinate systems: `position` in
  window coordinates (the unit of `logical_size()`), `pixel_position` in
  framebuffer pixels (the unit of `pixel_size()`).

## Input

### Keys are positions

`Key` names a **physical key**, after its label on a US QWERTY keyboard:
`Key::Q` is the key where QWERTY has Q, which AZERTY labels A and QWERTZ
labels Q. Bindings therefore keep their shape on every layout: a camera on
the keys under the left hand, or a hotkey grid, works unchanged in France
and Germany. To show a binding to the player, use
`Platform::key_label(key)`, which returns the label on their current layout
(`"A"` for `Key::Q` on AZERTY, `"Left Shift"` for `Key::LeftShift`).

Text entry (typing a player name) is not key input and is out of scope here.

### Input state

`InputState` turns the event stream into per-frame state. Each frame, call
`begin_frame()` and then `apply()` every event `poll_events()` returned:

| Query | True when |
|---|---|
| `down(key)` / `down(button)` | it is held |
| `pressed(key)` / `pressed(button)` | it went down this frame |
| `released(key)` / `released(button)` | it went up this frame |

A key pressed and released within one frame reports both `pressed` and
`released` and is not `down`, so a quick tap is never lost. A press of
something already down (its release was lost, e.g. while another window had
focus) is not a new edge, and neither is a release of something not down.

`modifiers()` reports Shift, Ctrl, Alt and Super, left and right combined,
from the keys held. `cursor()` is the latest `Cursor` from motion or a mouse
button, and empty once the cursor leaves its window (until the next motion).
`wheel()` sums this frame's `MouseWheel` deltas.

### Actions

Game code asks about **actions**, not devices. An `ActionMap` binds action
names to `Chord`s — a key or mouse button plus the modifiers held with it —
and answers `held`, `triggered` (went down this frame) and `ended` (went up
this frame) against an `InputState`.

- **Modifiers match exactly.** `Digit1` and `Ctrl+Digit1` are different
  chords, so "select group 1" and "assign group 1" never fire together.
- **Modifiers are read at query time.** `held`, `triggered` and `ended` all
  compare against the modifiers held *now*. A player who releases Shift
  before the mouse button ends the plain click chord, not Shift+click; a
  game that wants a drag to keep the modifiers it started with records them
  when the action triggers.
- A chord whose input is itself a modifier key ignores that modifier:
  `Key::LeftShift` bound alone matches while left Shift is held (and still
  rejects any *other* modifier).
- An action may have several chords; it is active when any of them is.
  Binding a chord the action already has does nothing.
- Bindings change at runtime: `unbind(action)` removes an action's chords and
  `bind` adds new ones. An action with no chords is never active.

### Edge scrolling

`edge_scroll(position, window_size, margin)` gives the camera direction for a
cursor resting near a window edge: each axis is -1, 0 or +1 (y grows
downwards). The cursor scrolls within `margin` of an edge (the boundary
counts as scrolling); a margin of 0 or less, or a position outside the
window, never scrolls. All arguments are in window coordinates.

### Example

```cpp
#include <opus/opus.hpp>

int main() {
    auto platform = opus::Platform::create();
    if (!platform) {
        return 1;
    }
    auto window = platform->create_window({.title = "Skirmish"});
    if (!window) {
        return 1;
    }

    opus::ActionMap actions;
    actions.bind("select", {.input = opus::MouseButton::Left});
    actions.bind("add_to_selection", {.input = opus::MouseButton::Left, .modifiers = {.shift = true}});
    actions.bind("assign_group_1", {.input = opus::Key::Digit1, .modifiers = {.ctrl = true}});
    actions.bind("quit", {.input = opus::Key::Escape});

    constexpr float edge_margin = 8.0F;
    opus::InputState input;
    for (bool running = true; running;) {
        input.begin_frame();
        for (const opus::Event& event : platform->poll_events()) {
            input.apply(event);
        }
        if (actions.triggered(input, "quit")) {
            running = false;
        }
        if (const auto cursor = input.cursor()) {
            const opus::EdgeScroll scroll =
                opus::edge_scroll(cursor->position, window->logical_size(), edge_margin);
            static_cast<void>(scroll); // move the camera by scroll.x, scroll.y
        }
    }
    return 0;
}
```

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
checks. `InputState` and `ActionMap` need no SDL: their specs feed them
synthetic `opus::Event` values directly.
