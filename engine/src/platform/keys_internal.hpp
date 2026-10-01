#pragma once

#include <opus/platform/platform.hpp>

#include <SDL3/SDL_scancode.h>

namespace opus::detail {

// Key ↔ SDL scancode, both directions; Key::Unknown and SDL_SCANCODE_UNKNOWN
// for anything without a counterpart.
Key key_from_scancode(SDL_Scancode scancode);
SDL_Scancode scancode_from_key(Key key);

} // namespace opus::detail
