#pragma once

#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

namespace opus::detail {

// Current sizes of the SDL window named by an id; zero when no such window
// exists (destroyed, moved-from, or the video subsystem is down).
Size logical_size_of(WindowId id);
Size pixel_size_of(WindowId id);
float display_scale_of(WindowId id);

// A cursor over the window, with its window-coordinate position converted to
// framebuffer pixels by the window's current pixel-to-logical ratio.
Cursor cursor_at(WindowId id, Point position);

} // namespace opus::detail
