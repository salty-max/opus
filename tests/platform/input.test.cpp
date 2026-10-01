#include <opus/platform/input.hpp>
#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include <doctest/doctest.h>

#include <initializer_list>
#include <optional>

using opus::Cursor;
using opus::CursorEntered;
using opus::CursorLeft;
using opus::edge_scroll;
using opus::EdgeScroll;
using opus::Event;
using opus::InputState;
using opus::Key;
using opus::KeyPressed;
using opus::KeyReleased;
using opus::Modifiers;
using opus::MouseButton;
using opus::MouseButtonPressed;
using opus::MouseButtonReleased;
using opus::MouseMoved;
using opus::MouseWheel;
using opus::Point;
using opus::Size;
using opus::WindowId;

namespace {

constexpr WindowId main_window{1};
constexpr WindowId other_window{2};
constexpr Cursor hover{
    .window = main_window, .position = {.x = 10.0F, .y = 20.0F}, .pixel_position = {.x = 20.0F, .y = 40.0F}};
constexpr Cursor click{
    .window = main_window, .position = {.x = 30.0F, .y = 5.0F}, .pixel_position = {.x = 60.0F, .y = 10.0F}};

constexpr Point small_scroll{.x = 0.5F, .y = 1.0F};
constexpr Point large_scroll{.x = 0.0F, .y = 2.0F};

constexpr Size screen{.width = 800, .height = 600};
constexpr float margin = 8.0F;
constexpr float right = static_cast<float>(screen.width);
constexpr float bottom = static_cast<float>(screen.height);
constexpr Point centre{.x = right / 2, .y = bottom / 2};

// One frame: forget the previous frame's edges, then apply its events.
void frame(InputState& input, std::initializer_list<Event> events) {
    input.begin_frame();
    for (const Event& event : events) {
        input.apply(event);
    }
}

} // namespace

TEST_CASE("InputState: starts with nothing held and no cursor") {
    const InputState input;
    CHECK_FALSE(input.down(Key::A));
    CHECK_FALSE(input.down(MouseButton::Left));
    CHECK(input.modifiers() == Modifiers{});
    CHECK_FALSE(input.cursor().has_value());
    CHECK(input.wheel() == Point{});
}

TEST_CASE("InputState: a key press is an edge for one frame and held until released") {
    InputState input;
    frame(input, {KeyPressed{.key = Key::W}});
    CHECK(input.pressed(Key::W));
    CHECK(input.down(Key::W));
    CHECK_FALSE(input.released(Key::W));

    frame(input, {});
    CHECK_FALSE(input.pressed(Key::W));
    CHECK(input.down(Key::W));

    frame(input, {KeyReleased{.key = Key::W}});
    CHECK(input.released(Key::W));
    CHECK_FALSE(input.down(Key::W));

    frame(input, {});
    CHECK_FALSE(input.released(Key::W));
}

TEST_CASE("InputState: a press and release within one frame report both edges") {
    InputState input;
    frame(input, {KeyPressed{.key = Key::Space}, KeyReleased{.key = Key::Space}});
    CHECK(input.pressed(Key::Space));
    CHECK(input.released(Key::Space));
    CHECK_FALSE(input.down(Key::Space));
}

TEST_CASE("InputState: a repeated press without a release is not a new edge") {
    InputState input;
    frame(input, {KeyPressed{.key = Key::E}});
    frame(input, {KeyPressed{.key = Key::E}});
    CHECK_FALSE(input.pressed(Key::E));
    CHECK(input.down(Key::E));
}

TEST_CASE("InputState: a release of a key that was not down is not an edge") {
    InputState input;
    frame(input, {KeyReleased{.key = Key::R}});
    CHECK_FALSE(input.released(Key::R));
}

TEST_CASE("InputState: tracks mouse buttons and moves the cursor to the click") {
    InputState input;
    frame(input, {MouseButtonPressed{.button = MouseButton::Right, .cursor = click}});
    CHECK(input.pressed(MouseButton::Right));
    CHECK(input.down(MouseButton::Right));
    CHECK(input.cursor() == click);

    frame(input, {MouseButtonReleased{.button = MouseButton::Right, .cursor = hover}});
    CHECK(input.released(MouseButton::Right));
    CHECK_FALSE(input.down(MouseButton::Right));
    CHECK(input.cursor() == hover);
}

TEST_CASE("InputState: a click within one frame reports both button edges") {
    InputState input;
    frame(input, {MouseButtonPressed{.button = MouseButton::Left, .cursor = click},
                  MouseButtonReleased{.button = MouseButton::Left, .cursor = click}});
    CHECK(input.pressed(MouseButton::Left));
    CHECK(input.released(MouseButton::Left));
    CHECK_FALSE(input.down(MouseButton::Left));
}

TEST_CASE("InputState: modifiers combine left and right keys") {
    InputState input;
    frame(input, {KeyPressed{.key = Key::RightShift}, KeyPressed{.key = Key::LeftCtrl}});
    CHECK(input.modifiers() == Modifiers{.shift = true, .ctrl = true});
    frame(input, {KeyPressed{.key = Key::LeftAlt}, KeyPressed{.key = Key::RightSuper}});
    CHECK(input.modifiers() == Modifiers{.shift = true, .ctrl = true, .alt = true, .super = true});
    frame(input, {KeyReleased{.key = Key::RightShift}});
    CHECK_FALSE(input.modifiers().shift);
}

TEST_CASE("InputState: the cursor follows motion and disappears when it leaves its window") {
    InputState input;
    frame(input, {CursorEntered{.window = main_window}});
    CHECK_FALSE(input.cursor().has_value());

    frame(input, {MouseMoved{.cursor = hover}});
    CHECK(input.cursor() == hover);

    frame(input, {CursorLeft{.window = other_window}});
    CHECK(input.cursor() == hover);

    frame(input, {CursorLeft{.window = main_window}});
    CHECK_FALSE(input.cursor().has_value());
}

TEST_CASE("InputState: the wheel accumulates within a frame and resets with the next") {
    InputState input;
    frame(input, {MouseWheel{.window = main_window, .delta = small_scroll},
                  MouseWheel{.window = main_window, .delta = large_scroll}});
    CHECK(input.wheel() == Point{.x = small_scroll.x + large_scroll.x, .y = small_scroll.y + large_scroll.y});
    frame(input, {});
    CHECK(input.wheel() == Point{});
}

TEST_CASE("InputState::apply: ignores events that are not input") {
    InputState input;
    frame(input, {opus::QuitRequested{}, opus::WindowCloseRequested{.window = main_window}});
    CHECK(input.modifiers() == Modifiers{});
    CHECK_FALSE(input.cursor().has_value());
}

TEST_CASE("edge_scroll: scrolls towards the edge the cursor is within the margin of") {
    CHECK(edge_scroll(centre, screen, margin) == EdgeScroll{});
    CHECK(edge_scroll({.x = 0.0F, .y = centre.y}, screen, margin) == EdgeScroll{.x = -1, .y = 0});
    CHECK(edge_scroll({.x = right - 1.0F, .y = centre.y}, screen, margin) == EdgeScroll{.x = 1, .y = 0});
    CHECK(edge_scroll({.x = centre.x, .y = 0.0F}, screen, margin) == EdgeScroll{.x = 0, .y = -1});
    CHECK(edge_scroll({.x = 0.0F, .y = bottom - 1.0F}, screen, margin) == EdgeScroll{.x = -1, .y = 1});
}

TEST_CASE("edge_scroll: the margin boundary belongs to the scrolling side") {
    CHECK(edge_scroll({.x = margin - 1.0F, .y = centre.y}, screen, margin).x == -1);
    CHECK(edge_scroll({.x = margin, .y = centre.y}, screen, margin).x == 0);
    CHECK(edge_scroll({.x = right - margin, .y = centre.y}, screen, margin).x == 1);
    CHECK(edge_scroll({.x = right - margin - 1.0F, .y = centre.y}, screen, margin).x == 0);
}

TEST_CASE("edge_scroll: never scrolls with no margin or a cursor outside the window") {
    CHECK(edge_scroll({.x = 0.0F, .y = 0.0F}, screen, 0.0F) == EdgeScroll{});
    CHECK(edge_scroll({.x = 0.0F, .y = 0.0F}, screen, -margin) == EdgeScroll{});
    CHECK(edge_scroll({.x = -1.0F, .y = centre.y}, screen, margin) == EdgeScroll{});
    CHECK(edge_scroll({.x = right, .y = centre.y}, screen, margin) == EdgeScroll{});
    CHECK(edge_scroll({.x = centre.x, .y = bottom}, screen, margin) == EdgeScroll{});
}
