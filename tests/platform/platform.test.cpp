#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include "platform/util.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include <doctest/doctest.h>

#include <optional>
#include <span>
#include <utility>
#include <vector>

using opus::Cursor;
using opus::CursorEntered;
using opus::CursorLeft;
using opus::Event;
using opus::Key;
using opus::KeyPressed;
using opus::KeyReleased;
using opus::MouseButton;
using opus::MouseButtonPressed;
using opus::MouseButtonReleased;
using opus::MouseMoved;
using opus::MouseWheel;
using opus::Platform;
using opus::PlatformError;
using opus::Point;
using opus::QuitRequested;
using opus::Size;
using opus::Window;
using opus::WindowCloseRequested;
using opus::WindowError;
using opus::WindowId;
using opus::WindowResized;
using opus::WindowScaleChanged;
using opus::test::headless_platform;
using opus::test::make_window;
using opus::test::sdl_window;

namespace {

// Arbitrary distinct sizes; each spec only needs them to differ.
constexpr Size first_size{.width = 800, .height = 600};
constexpr Size final_size{.width = 1024, .height = 768};
constexpr Size other_size{.width = 300, .height = 200};

std::vector<Event> poll(Platform& platform) {
    const std::span<const Event> events = platform.poll_events();
    return {events.begin(), events.end()};
}

// Delivers a window event the way the operating system would.
void push_window_event(SDL_EventType type, WindowId window) {
    SDL_Event event{};
    event.window.type = type;
    event.window.windowID = static_cast<SDL_WindowID>(window);
    REQUIRE(SDL_PushEvent(&event));
}

void push_key(SDL_Scancode scancode, bool down, bool repeat = false) {
    SDL_Event event{};
    event.key.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.scancode = scancode;
    event.key.down = down;
    event.key.repeat = repeat;
    REQUIRE(SDL_PushEvent(&event));
}

void push_mouse_button(const Window& window, Uint8 button, bool down, Point at) {
    SDL_Event event{};
    event.button.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    event.button.windowID = static_cast<SDL_WindowID>(window.id());
    event.button.button = button;
    event.button.down = down;
    event.button.x = at.x;
    event.button.y = at.y;
    REQUIRE(SDL_PushEvent(&event));
}

void push_wheel(const Window& window, Point delta, SDL_MouseWheelDirection direction) {
    SDL_Event event{};
    event.wheel.type = SDL_EVENT_MOUSE_WHEEL;
    event.wheel.windowID = static_cast<SDL_WindowID>(window.id());
    event.wheel.x = delta.x;
    event.wheel.y = delta.y;
    event.wheel.direction = direction;
    REQUIRE(SDL_PushEvent(&event));
}

// On the dummy display window coordinates and pixels coincide.
Cursor cursor_at(const Window& window, Point at) {
    return {.window = window.id(), .position = at, .pixel_position = at};
}

constexpr Point click_point{.x = 12.5F, .y = 40.0F};

void resize(const Window& window, Size size) {
    REQUIRE(SDL_SetWindowSize(sdl_window(window), size.width, size.height));
}

} // namespace

TEST_CASE("Platform::create: starts the video subsystem") {
    const Platform platform = headless_platform();
    CHECK(SDL_WasInit(SDL_INIT_VIDEO) != 0);
}

TEST_CASE("Platform::create: rejects a second live platform") {
    const Platform first = headless_platform();
    CHECK(Platform::create().error() == PlatformError::AlreadyInitialized);
}

TEST_CASE("Platform::create: reports a missing video driver") {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "no-such-driver");
    CHECK(Platform::create().error() == PlatformError::VideoUnavailable);
    CHECK(SDL_WasInit(SDL_INIT_VIDEO) == 0);
}

TEST_CASE("Platform: destruction shuts the video subsystem down") {
    {
        const Platform platform = headless_platform();
    }
    CHECK(SDL_WasInit(SDL_INIT_VIDEO) == 0);
    const Platform again = headless_platform();
    CHECK(SDL_WasInit(SDL_INIT_VIDEO) != 0);
}

TEST_CASE("Platform: a moved-from platform leaves the video subsystem running") {
    std::optional<Platform> source{headless_platform()};
    Platform target = *std::move(source);
    source.reset();
    CHECK(SDL_WasInit(SDL_INIT_VIDEO) != 0);
    CHECK(target.create_window({}).has_value());
}

TEST_CASE("Platform::create_window: a moved-from platform cannot create windows") {
    Platform source = headless_platform();
    const Platform target = std::move(source);
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move) moved-from state under test
    CHECK(source.create_window({}).error() == WindowError::PlatformInactive);
}

TEST_CASE("Platform: move assignment hands the connection to an empty platform") {
    Platform empty = headless_platform();
    Platform live = std::move(empty);
    empty = std::move(live);
    CHECK(SDL_WasInit(SDL_INIT_VIDEO) != 0);
    CHECK(empty.create_window({}).has_value());
}

TEST_CASE("Platform: assigning an empty platform shuts the video subsystem down") {
    Platform empty = headless_platform();
    Platform live = std::move(empty);
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move) moved-from state under test
    live = std::move(empty);
    CHECK(SDL_WasInit(SDL_INIT_VIDEO) == 0);
}

TEST_CASE("Platform::create_window: rejects zero and negative sizes") {
    Platform platform = headless_platform();
    CHECK(platform.create_window({.size = {.width = 0, .height = 480}}).error() == WindowError::InvalidSize);
    CHECK(platform.create_window({.size = {.width = 640, .height = -1}}).error() == WindowError::InvalidSize);
}

TEST_CASE("Platform::create_window: reports a window the operating system refuses") {
    Platform platform = headless_platform();
    // The video subsystem disappears underneath the platform and cannot come back.
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "no-such-driver");
    CHECK(platform.create_window({}).error() == WindowError::CreationFailed);
}

TEST_CASE("Platform::create_window: applies the configured window flags") {
    Platform platform = headless_platform();
    const auto flags_of = [](const Window& window) {
        return SDL_GetWindowFlags(sdl_window(window));
    };
    const Window defaults = make_window(platform);
    CHECK((flags_of(defaults) & SDL_WINDOW_RESIZABLE) != 0);
    CHECK((flags_of(defaults) & SDL_WINDOW_FULLSCREEN) == 0);
    CHECK((flags_of(defaults) & SDL_WINDOW_HIGH_PIXEL_DENSITY) != 0);

    auto fixed = platform.create_window({.resizable = false});
    REQUIRE(fixed.has_value());
    CHECK((flags_of(*fixed) & SDL_WINDOW_RESIZABLE) == 0);

    auto fullscreen = platform.create_window({.fullscreen = true});
    REQUIRE(fullscreen.has_value());
    CHECK((flags_of(*fullscreen) & SDL_WINDOW_FULLSCREEN) != 0);
}

TEST_CASE("Platform::poll_events: is empty when nothing happened") {
    Platform platform = headless_platform();
    static_cast<void>(poll(platform));
    CHECK(poll(platform).empty());
}

TEST_CASE("Platform::poll_events: reports a quit request") {
    Platform platform = headless_platform();
    static_cast<void>(poll(platform));
    SDL_Event quit{};
    quit.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit));
    CHECK(poll(platform) == std::vector<Event>{QuitRequested{}});
}

TEST_CASE("Platform::poll_events: reports a window close request") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    push_window_event(SDL_EVENT_WINDOW_CLOSE_REQUESTED, window.id());
    CHECK(poll(platform) == std::vector<Event>{WindowCloseRequested{.window = window.id()}});
}

TEST_CASE("Platform::poll_events: reports a resize with both sizes") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    resize(window, first_size);
    CHECK(poll(platform) == std::vector<Event>{WindowResized{
                                .window = window.id(),
                                .logical_size = first_size,
                                .pixel_size = first_size,
                            }});
}

TEST_CASE("Platform::poll_events: coalesces resizes of a window at its latest change") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    resize(window, first_size);
    push_window_event(SDL_EVENT_WINDOW_CLOSE_REQUESTED, window.id());
    resize(window, final_size);
    push_window_event(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, window.id());
    CHECK(poll(platform) == std::vector<Event>{
                                WindowCloseRequested{.window = window.id()},
                                WindowResized{
                                    .window = window.id(),
                                    .logical_size = final_size,
                                    .pixel_size = final_size,
                                },
                            });
}

TEST_CASE("Platform::poll_events: keeps resizes of different windows apart") {
    Platform platform = headless_platform();
    const Window first = make_window(platform);
    const Window second = make_window(platform);
    static_cast<void>(poll(platform));
    resize(first, other_size);
    resize(second, final_size);
    const std::vector<Event> events = poll(platform);
    REQUIRE(events.size() == 2);
    CHECK(std::get<WindowResized>(events[0]).window == first.id());
    CHECK(std::get<WindowResized>(events[1]).window == second.id());
}

TEST_CASE("Platform::poll_events: reports a display scale change with the new scale") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    push_window_event(SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED, window.id());
    CHECK(poll(platform) ==
          std::vector<Event>{WindowScaleChanged{.window = window.id(), .display_scale = 1.0F}});
}

TEST_CASE("Platform::poll_events: ignores events it does not translate") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    push_window_event(SDL_EVENT_WINDOW_SHOWN, window.id());
    CHECK(poll(platform).empty());
}

TEST_CASE("Platform::poll_events: reports key presses and releases by physical key") {
    Platform platform = headless_platform();
    static_cast<void>(poll(platform));
    push_key(SDL_SCANCODE_Q, true);
    push_key(SDL_SCANCODE_Q, false);
    push_key(SDL_SCANCODE_KP_ENTER, true);
    CHECK(poll(platform) == std::vector<Event>{KeyPressed{.key = Key::Q}, KeyReleased{.key = Key::Q},
                                               KeyPressed{.key = Key::KeypadEnter}});
}

TEST_CASE("Platform::poll_events: drops auto-repeat and keys without a Key") {
    Platform platform = headless_platform();
    static_cast<void>(poll(platform));
    push_key(SDL_SCANCODE_W, true, true);
    push_key(SDL_SCANCODE_MEDIA_PLAY, true);
    CHECK(poll(platform).empty());
}

TEST_CASE("Platform::poll_events: reports mouse buttons with their cursor") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    push_mouse_button(window, SDL_BUTTON_RIGHT, true, click_point);
    push_mouse_button(window, SDL_BUTTON_X2, false, click_point);
    CHECK(poll(platform) ==
          std::vector<Event>{
              MouseButtonPressed{.button = MouseButton::Right, .cursor = cursor_at(window, click_point)},
              MouseButtonReleased{.button = MouseButton::X2, .cursor = cursor_at(window, click_point)},
          });
}

TEST_CASE("Platform::poll_events: drops mouse buttons beyond the five named ones") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    constexpr Uint8 sixth_button = 6;
    push_mouse_button(window, sixth_button, true, click_point);
    CHECK(poll(platform).empty());
}

TEST_CASE("Platform::poll_events: reports cursor motion in both coordinate systems") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    SDL_WarpMouseInWindow(sdl_window(window), click_point.x, click_point.y);
    const std::vector<Event> events = poll(platform);
    REQUIRE_FALSE(events.empty());
    CHECK(events.back() == Event{MouseMoved{.cursor = cursor_at(window, click_point)}});
}

TEST_CASE("Platform::poll_events: normalises the wheel so positive y scrolls away from the user") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    push_wheel(window, {.x = 0.0F, .y = 1.0F}, SDL_MOUSEWHEEL_NORMAL);
    push_wheel(window, {.x = 0.0F, .y = -1.0F}, SDL_MOUSEWHEEL_FLIPPED);
    CHECK(poll(platform) == std::vector<Event>{
                                MouseWheel{.window = window.id(), .delta = {.x = 0.0F, .y = 1.0F}},
                                MouseWheel{.window = window.id(), .delta = {.x = 0.0F, .y = 1.0F}},
                            });
}

TEST_CASE("Platform::poll_events: reports the cursor entering and leaving a window") {
    Platform platform = headless_platform();
    const Window window = make_window(platform);
    static_cast<void>(poll(platform));
    push_window_event(SDL_EVENT_WINDOW_MOUSE_ENTER, window.id());
    push_window_event(SDL_EVENT_WINDOW_MOUSE_LEAVE, window.id());
    CHECK(poll(platform) ==
          std::vector<Event>{CursorEntered{.window = window.id()}, CursorLeft{.window = window.id()}});
}

TEST_CASE("Platform::key_label: names a key by its label on the current layout") {
    const Platform platform = headless_platform();
    // The dummy driver uses the US layout, where positions and labels agree.
    CHECK(platform.key_label(Key::Q) == "Q");
    CHECK(platform.key_label(Key::LeftShift) == "Left Shift");
    CHECK(platform.key_label(Key::Unknown).empty());
}

TEST_CASE("Platform::key_label: is empty on a moved-from platform") {
    Platform source = headless_platform();
    const Platform target = std::move(source);
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move) moved-from state under test
    CHECK(source.key_label(Key::Q).empty());
}
