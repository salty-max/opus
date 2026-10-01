#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include "platform/keys_internal.hpp"
#include "platform/window_internal.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace opus {

namespace {

WindowId window_of(const SDL_Event& event) {
    return WindowId{event.window.windowID};
}

std::optional<MouseButton> mouse_button_of(Uint8 button) {
    switch (button) {
    case SDL_BUTTON_LEFT:
        return MouseButton::Left;
    case SDL_BUTTON_MIDDLE:
        return MouseButton::Middle;
    case SDL_BUTTON_RIGHT:
        return MouseButton::Right;
    case SDL_BUTTON_X1:
        return MouseButton::X1;
    case SDL_BUTTON_X2:
        return MouseButton::X2;
    default:
        return std::nullopt;
    }
}

Cursor cursor_of(const SDL_MouseButtonEvent& event) {
    return detail::cursor_at(WindowId{event.windowID}, {.x = event.x, .y = event.y});
}

// Keys with no Key counterpart and auto-repeats are not reported.
void record_key(std::vector<Event>& events, const SDL_KeyboardEvent& event) {
    const Key key = detail::key_from_scancode(event.scancode);
    if (key == Key::Unknown || event.repeat) {
        return;
    }
    if (event.down) {
        events.emplace_back(KeyPressed{.key = key});
    } else {
        events.emplace_back(KeyReleased{.key = key});
    }
}

// Buttons beyond the five MouseButton names are not reported.
void record_mouse_button(std::vector<Event>& events, const SDL_MouseButtonEvent& event) {
    const std::optional<MouseButton> button = mouse_button_of(event.button);
    if (!button) {
        return;
    }
    if (event.down) {
        events.emplace_back(MouseButtonPressed{.button = *button, .cursor = cursor_of(event)});
    } else {
        events.emplace_back(MouseButtonReleased{.button = *button, .cursor = cursor_of(event)});
    }
}

MouseWheel wheel_of(const SDL_MouseWheelEvent& event) {
    // "Natural" scrolling reports inverted deltas; undo it so positive y
    // always scrolls away from the user.
    const float direction = event.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0F : 1.0F;
    return {
        .window = WindowId{event.windowID},
        .delta = {.x = event.x * direction, .y = event.y * direction},
    };
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
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            record_key(events_, event.key);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            record_mouse_button(events_, event.button);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            events_.emplace_back(
                MouseMoved{.cursor = detail::cursor_at(WindowId{event.motion.windowID},
                                                       {.x = event.motion.x, .y = event.motion.y})});
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            events_.emplace_back(wheel_of(event.wheel));
            break;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            events_.emplace_back(CursorEntered{.window = window_of(event)});
            break;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            events_.emplace_back(CursorLeft{.window = window_of(event)});
            break;
        default:
            break;
        }
    }
    return events_;
}

std::string Platform::key_label(Key key) const {
    const SDL_Scancode scancode = detail::scancode_from_key(key);
    // The layout is only known while the video subsystem runs.
    if (scancode == SDL_SCANCODE_UNKNOWN || !owns_video_) {
        return {};
    }
    return SDL_GetKeyName(SDL_GetKeyFromScancode(scancode, SDL_KMOD_NONE, false));
}

} // namespace opus
