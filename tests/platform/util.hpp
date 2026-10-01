#pragma once

#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_video.h>

#include <cstdint>

#include <doctest/doctest.h>

#include <format>
#include <string>
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

namespace opus::test::detail {

inline std::string text(Size s) {
    return std::format("{}x{}", s.width, s.height);
}
inline std::string text(Point p) {
    return std::format("({}, {})", p.x, p.y);
}
inline std::uint32_t text(WindowId w) {
    return static_cast<std::uint32_t>(w);
}
inline std::string text(const Cursor& c) {
    return std::format("{{{}, {}, {}}}", text(c.window), text(c.position), text(c.pixel_position));
}
inline std::string text(const QuitRequested& /*event*/) {
    return "QuitRequested{}";
}
inline std::string text(const WindowCloseRequested& e) {
    return std::format("WindowCloseRequested{{{}}}", text(e.window));
}
inline std::string text(const WindowResized& e) {
    return std::format("WindowResized{{{}, {}, {}}}", text(e.window), text(e.logical_size),
                       text(e.pixel_size));
}
inline std::string text(const WindowScaleChanged& e) {
    return std::format("WindowScaleChanged{{{}, {}}}", text(e.window), e.display_scale);
}
inline std::string text(const KeyPressed& e) {
    return std::format("KeyPressed{{{}}}", static_cast<int>(e.key));
}
inline std::string text(const KeyReleased& e) {
    return std::format("KeyReleased{{{}}}", static_cast<int>(e.key));
}
inline std::string text(const MouseButtonPressed& e) {
    return std::format("MouseButtonPressed{{{}, {}}}", static_cast<int>(e.button), text(e.cursor));
}
inline std::string text(const MouseButtonReleased& e) {
    return std::format("MouseButtonReleased{{{}, {}}}", static_cast<int>(e.button), text(e.cursor));
}
inline std::string text(const MouseMoved& e) {
    return std::format("MouseMoved{{{}}}", text(e.cursor));
}
inline std::string text(const MouseWheel& e) {
    return std::format("MouseWheel{{{}, {}}}", text(e.window), text(e.delta));
}
inline std::string text(const CursorEntered& e) {
    return std::format("CursorEntered{{{}}}", text(e.window));
}
inline std::string text(const CursorLeft& e) {
    return std::format("CursorLeft{{{}}}", text(e.window));
}

} // namespace opus::test::detail

/// Prints platform events in failed checks, e.g. `WindowResized{3, 800x600, 800x600}`.
template <> struct doctest::StringMaker<opus::Event> {
    static doctest::String convert(const opus::Event& event) {
        const std::string text = std::visit([](const auto& e) { return opus::test::detail::text(e); }, event);
        return {text.c_str()};
    }
};
