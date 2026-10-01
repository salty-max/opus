#pragma once

#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_video.h>

#include <cstdint>

#include <doctest/doctest.h>

#include <format>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace opus::test {

/// A Platform on SDL's dummy video driver, so platform specs need no display
/// and see the same 1.0 display scale on every machine.
///
/// The hint is set on every call: SDL forgets hints when it shuts down.
inline Platform headless_platform() {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    auto platform = Platform::create();
    REQUIRE(platform.has_value());
    return *std::move(platform);
}

/// A window from @p platform; fails the spec if creation fails.
inline Window make_window(const Platform& platform, Size size = {.width = 640, .height = 480}) {
    auto window = platform.create_window({.title = "spec", .size = size});
    REQUIRE(window.has_value());
    return *std::move(window);
}

/// The SDL window behind @p window, for specs that act as the operating
/// system; null once the window is gone.
inline SDL_Window* sdl_window(const Window& window) {
    return SDL_GetWindowFromID(static_cast<SDL_WindowID>(window.id()));
}

} // namespace opus::test

/// Prints platform events in failed checks, e.g. `WindowResized{3, 800x600, 800x600}`.
template <> struct doctest::StringMaker<opus::Event> {
    static doctest::String convert(const opus::Event& event) {
        const auto size = [](opus::Size s) {
            return std::format("{}x{}", s.width, s.height);
        };
        const auto id = [](opus::WindowId w) {
            return static_cast<std::uint32_t>(w);
        };
        const std::string text = std::visit(
            [&](const auto& e) -> std::string {
                using E = std::decay_t<decltype(e)>;
                if constexpr (std::is_same_v<E, opus::QuitRequested>) {
                    return "QuitRequested{}";
                } else if constexpr (std::is_same_v<E, opus::WindowCloseRequested>) {
                    return std::format("WindowCloseRequested{{{}}}", id(e.window));
                } else if constexpr (std::is_same_v<E, opus::WindowResized>) {
                    return std::format("WindowResized{{{}, {}, {}}}", id(e.window), size(e.logical_size),
                                       size(e.pixel_size));
                } else {
                    return std::format("WindowScaleChanged{{{}, {}}}", id(e.window), e.display_scale);
                }
            },
            event);
        return {text.c_str()};
    }
};
