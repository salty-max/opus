#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include "platform/window_internal.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <expected>
#include <span>
#include <utility>
#include <variant>
#include <vector>

namespace opus {

namespace {

WindowId window_of(const SDL_Event& event) {
    return WindowId{event.window.windowID};
}

// Coalesces size changes: a window's earlier WindowResized is dropped and the
// final sizes are reported at the position of its latest change. SDL already
// drops superseded resize events, so this keeps the same ordering.
void record_resize(std::vector<Event>& events, WindowId window) {
    std::erase_if(events, [window](const Event& event) {
        const auto* earlier = std::get_if<WindowResized>(&event);
        return earlier != nullptr && earlier->window == window;
    });
    events.emplace_back(WindowResized{
        .window = window,
        .logical_size = detail::logical_size_of(window),
        .pixel_size = detail::pixel_size_of(window),
    });
}

} // namespace

std::expected<Platform, PlatformError> Platform::create() {
    if (SDL_WasInit(SDL_INIT_VIDEO) != 0) {
        return std::unexpected{PlatformError::AlreadyInitialized};
    }
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return std::unexpected{PlatformError::VideoUnavailable};
    }
    Platform platform;
    platform.owns_video_ = true;
    return platform;
}

Platform::Platform(Platform&& other) noexcept
    : owns_video_{std::exchange(other.owns_video_, false)}, events_{std::move(other.events_)} {
}

Platform& Platform::operator=(Platform&& other) noexcept {
    if (this != &other) {
        // The connection being replaced shuts down with `discarded`.
        const Platform discarded{std::move(*this)};
        owns_video_ = std::exchange(other.owns_video_, false);
        events_ = std::move(other.events_);
    }
    return *this;
}

Platform::~Platform() {
    if (owns_video_) {
        SDL_Quit();
    }
}

std::expected<Window, WindowError> Platform::create_window(const WindowConfig& config) const {
    if (config.size.width <= 0 || config.size.height <= 0) {
        return std::unexpected{WindowError::InvalidSize};
    }
    // SDL would quietly start the video subsystem again; only a live platform may.
    if (!owns_video_) {
        return std::unexpected{WindowError::PlatformInactive};
    }
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (config.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    if (config.fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    SDL_Window* window = SDL_CreateWindow(config.title.c_str(), config.size.width, config.size.height, flags);
    if (window == nullptr) {
        return std::unexpected{WindowError::CreationFailed};
    }
    return Window{WindowId{SDL_GetWindowID(window)}};
}

std::span<const Event> Platform::poll_events() {
    events_.clear();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            events_.emplace_back(QuitRequested{});
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            events_.emplace_back(WindowCloseRequested{.window = window_of(event)});
            break;
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            record_resize(events_, window_of(event));
            break;
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            events_.emplace_back(WindowScaleChanged{
                .window = window_of(event),
                .display_scale = detail::display_scale_of(window_of(event)),
            });
            break;
        default:
            break;
        }
    }
    return events_;
}

} // namespace opus
