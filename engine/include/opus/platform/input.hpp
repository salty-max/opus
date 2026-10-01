#pragma once

#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include <bitset>
#include <cstdint>
#include <optional>

namespace opus {

/// Which modifier keys are held; left and right count the same.
struct Modifiers {
    bool shift = false; ///< Either Shift.
    bool ctrl = false;  ///< Either Control.
    bool alt = false;   ///< Either Alt (Option on macOS).
    bool super = false; ///< Either Windows/Command key.

    /// Memberwise equality.
    friend bool operator==(Modifiers, Modifiers) = default;
};

/// Keyboard and mouse state, built from platform events one frame at a time.
///
/// Each frame: call begin_frame(), then apply() every event the platform
/// reported. A key pressed and released within one frame reports both
/// pressed() and released(), and is not down().
///
/// @code
/// input.begin_frame();
/// for (const opus::Event& event : platform.poll_events()) {
///     input.apply(event);
/// }
/// if (input.pressed(opus::Key::Escape)) { /* ... */ }
/// @endcode
class InputState {
public:
    /// Forgets this frame's edges (pressed, released) and wheel motion.
    void begin_frame();

    /// Updates the state from @p event; events other than input are ignored.
    void apply(const Event& event);

    /// Whether @p key is held.
    [[nodiscard]] bool down(Key key) const;
    /// Whether @p key went down this frame.
    [[nodiscard]] bool pressed(Key key) const;
    /// Whether @p key went up this frame.
    [[nodiscard]] bool released(Key key) const;

    /// Whether @p button is held.
    [[nodiscard]] bool down(MouseButton button) const;
    /// Whether @p button went down this frame.
    [[nodiscard]] bool pressed(MouseButton button) const;
    /// Whether @p button went up this frame.
    [[nodiscard]] bool released(MouseButton button) const;

    /// The modifier keys currently held.
    [[nodiscard]] Modifiers modifiers() const;

    /// Where the cursor is; empty while it is outside every window.
    [[nodiscard]] std::optional<Cursor> cursor() const;

    /// Wheel motion accumulated this frame, in MouseWheel's units.
    [[nodiscard]] Point wheel() const;

private:
    struct Edges {
        std::bitset<key_count> down;
        std::bitset<key_count> pressed;
        std::bitset<key_count> released;
    };
    struct ButtonEdges {
        std::bitset<mouse_button_count> down;
        std::bitset<mouse_button_count> pressed;
        std::bitset<mouse_button_count> released;
    };

    Edges keys_;
    ButtonEdges buttons_;
    std::optional<Cursor> cursor_;
    Point wheel_;
};

/// Direction to scroll the camera when the cursor rests near a window edge:
/// each component is -1, 0 or +1 (x grows rightwards, y downwards).
struct EdgeScroll {
    std::int8_t x = 0; ///< -1 at the left edge, +1 at the right edge, else 0.
    std::int8_t y = 0; ///< -1 at the top edge, +1 at the bottom edge, else 0.

    /// Memberwise equality.
    friend bool operator==(EdgeScroll, EdgeScroll) = default;
};

/// Edge-scroll direction for a cursor at @p position in a window of
/// @p window_size, both in window coordinates. The cursor scrolls when it is
/// within @p margin of an edge; a margin of 0 or less never scrolls. A
/// position outside the window never scrolls.
[[nodiscard]] EdgeScroll edge_scroll(Point position, Size window_size, float margin);

} // namespace opus
