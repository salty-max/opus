#include <opus/platform/window.hpp>

#include "platform/util.hpp"

#include <SDL3/SDL_video.h>

#include <doctest/doctest.h>

#include <optional>
#include <utility>

using opus::Platform;
using opus::Size;
using opus::Window;
using opus::WindowId;
using opus::test::headless_platform;
using opus::test::make_window;

namespace {

bool os_window_exists(WindowId id) {
    return SDL_GetWindowFromID(static_cast<SDL_WindowID>(id)) != nullptr;
}

} // namespace

TEST_CASE("Window: reports its sizes and scale") {
    Platform platform = headless_platform();
    const Window window = make_window(platform, {.width = 320, .height = 240});
    CHECK(window.logical_size() == Size{.width = 320, .height = 240});
    // The dummy display has a scale of 1, so pixels and window coordinates agree.
    CHECK(window.pixel_size() == Size{.width = 320, .height = 240});
    CHECK(window.display_scale() == 1.0F);
}

TEST_CASE("Window::id: names each window distinctly and never with zero") {
    Platform platform = headless_platform();
    const Window first = make_window(platform);
    const Window second = make_window(platform);
    CHECK(first.id() != WindowId{});
    CHECK(second.id() != WindowId{});
    CHECK(first.id() != second.id());
}

TEST_CASE("Window: destruction closes the operating-system window") {
    Platform platform = headless_platform();
    std::optional<Window> window = make_window(platform);
    const WindowId id = window->id();
    REQUIRE(os_window_exists(id));
    window.reset();
    CHECK_FALSE(os_window_exists(id));
}

TEST_CASE("Window: moving transfers the operating-system window") {
    Platform platform = headless_platform();
    Window source = make_window(platform);
    const WindowId id = source.id();
    const Window target = std::move(source);
    CHECK(target.id() == id);
    // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move) moved-from state under test
    CHECK(source.id() == WindowId{});
    CHECK(os_window_exists(id));
}

TEST_CASE("Window: move assignment closes the window it replaces") {
    Platform platform = headless_platform();
    Window target = make_window(platform);
    Window source = make_window(platform);
    const WindowId replaced = target.id();
    const WindowId kept = source.id();
    target = std::move(source);
    CHECK(target.id() == kept);
    CHECK_FALSE(os_window_exists(replaced));
    CHECK(os_window_exists(kept));
}

TEST_CASE("Window: a moved-from window reports zero sizes") {
    Platform platform = headless_platform();
    Window source = make_window(platform);
    const Window target = std::move(source);
    // NOLINTBEGIN(bugprone-use-after-move,clang-analyzer-cplusplus.Move) moved-from state under test
    CHECK(source.logical_size() == Size{});
    CHECK(source.pixel_size() == Size{});
    CHECK(source.display_scale() == 0.0F);
    // NOLINTEND(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
}

TEST_CASE("Window: outliving its platform reports zero sizes and is safe to destroy") {
    std::optional<Window> window;
    {
        Platform platform = headless_platform();
        window = make_window(platform);
    }
    CHECK(window->logical_size() == Size{});
    CHECK(window->display_scale() == 0.0F);
    window.reset();
}
