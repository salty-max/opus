#pragma once

#include <opus/platform/input.hpp>
#include <opus/platform/platform.hpp>

#include <functional>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace opus {

/// A key or mouse button together with the exact modifiers held with it,
/// e.g. Ctrl+Digit1 or Shift+left click.
///
/// Modifiers must match exactly, so Digit1 and Ctrl+Digit1 can be bound to
/// different actions. A chord whose input is itself a modifier key ignores
/// that modifier: Key::LeftShift alone matches while left Shift is held.
struct Chord {
    std::variant<Key, MouseButton> input; ///< The key or button that triggers the chord.
    Modifiers modifiers{};                ///< Modifiers that must be held, and no others.

    /// Memberwise equality.
    friend bool operator==(const Chord&, const Chord&) = default;
};

/// Named actions bound to chords, so game code asks "is `select` held?"
/// rather than "is the left button down?". Bindings can change at any time.
///
/// An action may have several chords; it is active when any of them is.
/// Querying an action with no chords (including one never bound) is false.
/// Every query compares against the modifiers held at the time of the query:
/// releasing Shift before the mouse button ends the unshifted chord, not the
/// shifted one.
///
/// @code
/// opus::ActionMap actions;
/// actions.bind("assign_group_1", {.input = opus::Key::Digit1, .modifiers = {.ctrl = true}});
/// if (actions.triggered(input, "assign_group_1")) { /* ... */ }
/// @endcode
class ActionMap {
public:
    /// Adds @p chord to @p action's chords; binding a chord it already has does nothing.
    void bind(std::string_view action, const Chord& chord);

    /// Removes every chord of @p action.
    void unbind(std::string_view action);

    /// The chords bound to @p action, in binding order; empty if none.
    [[nodiscard]] std::span<const Chord> chords(std::string_view action) const;

    /// Whether any of @p action's chords is held.
    [[nodiscard]] bool held(const InputState& input, std::string_view action) const;

    /// Whether any of @p action's chords went down this frame.
    [[nodiscard]] bool triggered(const InputState& input, std::string_view action) const;

    /// Whether any of @p action's chords went up this frame.
    [[nodiscard]] bool ended(const InputState& input, std::string_view action) const;

private:
    std::map<std::string, std::vector<Chord>, std::less<>> bindings_;
};

} // namespace opus
