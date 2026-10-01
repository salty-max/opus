#include <opus/platform/actions.hpp>
#include <opus/platform/input.hpp>
#include <opus/platform/platform.hpp>

#include <doctest/doctest.h>

#include <initializer_list>
#include <vector>

using opus::ActionMap;
using opus::Chord;
using opus::Cursor;
using opus::Event;
using opus::InputState;
using opus::Key;
using opus::KeyPressed;
using opus::KeyReleased;
using opus::MouseButton;
using opus::MouseButtonPressed;

namespace {

void frame(InputState& input, std::initializer_list<Event> events) {
    input.begin_frame();
    for (const Event& event : events) {
        input.apply(event);
    }
}

const Chord select_group_1{.input = Key::Digit1, .modifiers = {}};
const Chord assign_group_1{.input = Key::Digit1, .modifiers = {.ctrl = true}};

} // namespace

TEST_CASE("ActionMap: an unbound action is never active") {
    const ActionMap actions;
    InputState input;
    frame(input, {KeyPressed{.key = Key::A}});
    CHECK(actions.chords("attack").empty());
    CHECK_FALSE(actions.held(input, "attack"));
    CHECK_FALSE(actions.triggered(input, "attack"));
    CHECK_FALSE(actions.ended(input, "attack"));
}

TEST_CASE("ActionMap: follows its chord's key through held, triggered and ended") {
    ActionMap actions;
    actions.bind("attack", {.input = Key::A});
    InputState input;

    frame(input, {KeyPressed{.key = Key::A}});
    CHECK(actions.triggered(input, "attack"));
    CHECK(actions.held(input, "attack"));

    frame(input, {KeyReleased{.key = Key::A}});
    CHECK(actions.ended(input, "attack"));
    CHECK_FALSE(actions.held(input, "attack"));
}

TEST_CASE("ActionMap: binds mouse buttons") {
    ActionMap actions;
    actions.bind("select", {.input = MouseButton::Left});
    InputState input;
    frame(input, {MouseButtonPressed{.button = MouseButton::Left, .cursor = Cursor{}}});
    CHECK(actions.triggered(input, "select"));
}

TEST_CASE("ActionMap: requires the chord's modifiers exactly") {
    ActionMap actions;
    actions.bind("select_group_1", select_group_1);
    actions.bind("assign_group_1", assign_group_1);
    InputState input;

    frame(input, {KeyPressed{.key = Key::Digit1}});
    CHECK(actions.triggered(input, "select_group_1"));
    CHECK_FALSE(actions.triggered(input, "assign_group_1"));

    frame(input, {KeyReleased{.key = Key::Digit1}, KeyPressed{.key = Key::LeftCtrl},
                  KeyPressed{.key = Key::Digit1}});
    CHECK(actions.triggered(input, "assign_group_1"));
    CHECK_FALSE(actions.triggered(input, "select_group_1"));
}

TEST_CASE("ActionMap: a modifier key bound alone matches while it is held") {
    ActionMap actions;
    actions.bind("queue_orders", {.input = Key::LeftShift});
    InputState input;
    frame(input, {KeyPressed{.key = Key::LeftShift}});
    CHECK(actions.triggered(input, "queue_orders"));
    CHECK(actions.held(input, "queue_orders"));
    frame(input, {KeyReleased{.key = Key::LeftShift}});
    CHECK(actions.ended(input, "queue_orders"));
}

TEST_CASE("ActionMap: a modifier key bound alone still rejects other modifiers") {
    ActionMap actions;
    actions.bind("queue_orders", {.input = Key::LeftShift});
    InputState input;
    frame(input, {KeyPressed{.key = Key::LeftCtrl}, KeyPressed{.key = Key::LeftShift}});
    CHECK_FALSE(actions.triggered(input, "queue_orders"));
}

TEST_CASE("ActionMap: any of an action's chords activates it") {
    ActionMap actions;
    actions.bind("camera_left", {.input = Key::Left});
    actions.bind("camera_left", {.input = Key::A});
    InputState input;
    frame(input, {KeyPressed{.key = Key::A}});
    CHECK(actions.held(input, "camera_left"));
}

TEST_CASE("ActionMap::bind: ignores a chord the action already has") {
    ActionMap actions;
    actions.bind("attack", {.input = Key::A});
    actions.bind("attack", {.input = Key::A});
    CHECK(actions.chords("attack").size() == 1);
}

TEST_CASE("ActionMap::chords: lists an action's chords in binding order") {
    ActionMap actions;
    actions.bind("group_1", select_group_1);
    actions.bind("group_1", assign_group_1);
    const auto chords = actions.chords("group_1");
    CHECK(std::vector<Chord>{chords.begin(), chords.end()} ==
          std::vector<Chord>{select_group_1, assign_group_1});
}

TEST_CASE("ActionMap: rebinding at runtime replaces an action's chords") {
    ActionMap actions;
    actions.bind("attack", {.input = Key::A});
    actions.unbind("attack");
    actions.bind("attack", {.input = Key::Q});
    InputState input;

    frame(input, {KeyPressed{.key = Key::A}});
    CHECK_FALSE(actions.triggered(input, "attack"));
    frame(input, {KeyPressed{.key = Key::Q}});
    CHECK(actions.triggered(input, "attack"));
}

TEST_CASE("ActionMap::unbind: leaves other actions bound and accepts unknown names") {
    ActionMap actions;
    actions.bind("attack", {.input = Key::A});
    actions.bind("stop", {.input = Key::S});
    actions.unbind("attack");
    actions.unbind("never_bound");
    CHECK(actions.chords("attack").empty());
    CHECK(actions.chords("stop").size() == 1);
}

TEST_CASE("ActionMap: compares modifiers held at the time of the query") {
    ActionMap actions;
    actions.bind("select", {.input = MouseButton::Left});
    actions.bind("add_to_selection", {.input = MouseButton::Left, .modifiers = {.shift = true}});
    InputState input;

    frame(input, {KeyPressed{.key = Key::LeftShift},
                  MouseButtonPressed{.button = MouseButton::Left, .cursor = Cursor{}}});
    CHECK(actions.triggered(input, "add_to_selection"));
    CHECK_FALSE(actions.triggered(input, "select"));

    frame(input, {KeyReleased{.key = Key::LeftShift}});
    CHECK(actions.held(input, "select"));
    CHECK_FALSE(actions.held(input, "add_to_selection"));

    frame(input, {opus::MouseButtonReleased{.button = MouseButton::Left, .cursor = Cursor{}}});
    CHECK(actions.ended(input, "select"));
    CHECK_FALSE(actions.ended(input, "add_to_selection"));
}
