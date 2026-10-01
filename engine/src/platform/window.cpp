#include <opus/platform/window.hpp>

#include "platform/window_internal.hpp"

#include <SDL3/SDL_video.h>

#include <utility>

namespace opus {

namespace detail {

namespace {

SDL_Window* sdl_window(WindowId id) {
    return SDL_GetWindowFromID(static_cast<SDL_WindowID>(id));
}

} // namespace

Size logical_size_of(WindowId id) {
    Size size;
    if (SDL_Window* window = sdl_window(id);
        window == nullptr || !SDL_GetWindowSize(window, &size.width, &size.height)) {
        return {};
    }
    return size;
}

Size pixel_size_of(WindowId id) {
    Size size;
    if (SDL_Window* window = sdl_window(id);
        window == nullptr || !SDL_GetWindowSizeInPixels(window, &size.width, &size.height)) {
        return {};
    }
    return size;
}

float display_scale_of(WindowId id) {
    SDL_Window* window = sdl_window(id);
    // SDL reports 0 on failure; so does a window that no longer exists.
    return window == nullptr ? 0.0F : SDL_GetWindowDisplayScale(window);
}

} // namespace detail

Window::Window(WindowId id) : id_{id} {
}

Window::Window(Window&& other) noexcept : id_{std::exchange(other.id_, WindowId{})} {
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        // The window being replaced is destroyed with `discarded`.
        const Window discarded{std::exchange(id_, std::exchange(other.id_, WindowId{}))};
    }
    return *this;
}

Window::~Window() {
    if (SDL_Window* window = SDL_GetWindowFromID(static_cast<SDL_WindowID>(id_)); window != nullptr) {
        SDL_DestroyWindow(window);
    }
}

WindowId Window::id() const {
    return id_;
}

Size Window::logical_size() const {
    return detail::logical_size_of(id_);
}

Size Window::pixel_size() const {
    return detail::pixel_size_of(id_);
}

float Window::display_scale() const {
    return detail::display_scale_of(id_);
}

} // namespace opus
