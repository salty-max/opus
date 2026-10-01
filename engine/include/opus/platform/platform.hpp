#pragma once

#include <opus/platform/window.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace opus {

/// A physical key, named after its label on a US QWERTY keyboard.
///
/// Keys identify positions, not characters: Key::Q is the key where QWERTY
/// has Q, which AZERTY labels A. Bindings therefore keep their layout on every
/// keyboard; Platform::key_label gives the label to show the user.
enum class Key : std::uint8_t {
    Unknown,        ///< A key with no Key counterpart; never reported.
    A,              ///< Letter key in the position of QWERTY A.
    B,              ///< Letter key in the position of QWERTY B.
    C,              ///< Letter key in the position of QWERTY C.
    D,              ///< Letter key in the position of QWERTY D.
    E,              ///< Letter key in the position of QWERTY E.
    F,              ///< Letter key in the position of QWERTY F.
    G,              ///< Letter key in the position of QWERTY G.
    H,              ///< Letter key in the position of QWERTY H.
    I,              ///< Letter key in the position of QWERTY I.
    J,              ///< Letter key in the position of QWERTY J.
    K,              ///< Letter key in the position of QWERTY K.
    L,              ///< Letter key in the position of QWERTY L.
    M,              ///< Letter key in the position of QWERTY M.
    N,              ///< Letter key in the position of QWERTY N.
    O,              ///< Letter key in the position of QWERTY O.
    P,              ///< Letter key in the position of QWERTY P.
    Q,              ///< Letter key in the position of QWERTY Q.
    R,              ///< Letter key in the position of QWERTY R.
    S,              ///< Letter key in the position of QWERTY S.
    T,              ///< Letter key in the position of QWERTY T.
    U,              ///< Letter key in the position of QWERTY U.
    V,              ///< Letter key in the position of QWERTY V.
    W,              ///< Letter key in the position of QWERTY W.
    X,              ///< Letter key in the position of QWERTY X.
    Y,              ///< Letter key in the position of QWERTY Y.
    Z,              ///< Letter key in the position of QWERTY Z.
    Digit0,         ///< Number-row key in the position of QWERTY 0.
    Digit1,         ///< Number-row key in the position of QWERTY 1.
    Digit2,         ///< Number-row key in the position of QWERTY 2.
    Digit3,         ///< Number-row key in the position of QWERTY 3.
    Digit4,         ///< Number-row key in the position of QWERTY 4.
    Digit5,         ///< Number-row key in the position of QWERTY 5.
    Digit6,         ///< Number-row key in the position of QWERTY 6.
    Digit7,         ///< Number-row key in the position of QWERTY 7.
    Digit8,         ///< Number-row key in the position of QWERTY 8.
    Digit9,         ///< Number-row key in the position of QWERTY 9.
    F1,             ///< Function key F1.
    F2,             ///< Function key F2.
    F3,             ///< Function key F3.
    F4,             ///< Function key F4.
    F5,             ///< Function key F5.
    F6,             ///< Function key F6.
    F7,             ///< Function key F7.
    F8,             ///< Function key F8.
    F9,             ///< Function key F9.
    F10,            ///< Function key F10.
    F11,            ///< Function key F11.
    F12,            ///< Function key F12.
    Escape,         ///< Escape.
    Enter,          ///< Main Enter/Return key.
    Tab,            ///< Tab.
    Backspace,      ///< Backspace.
    Space,          ///< Space bar.
    Minus,          ///< Key right of the number row's 0 (QWERTY -).
    Equals,         ///< Key left of Backspace (QWERTY =).
    LeftBracket,    ///< Key right of P (QWERTY [).
    RightBracket,   ///< Key right of LeftBracket (QWERTY ]).
    Backslash,      ///< Key above Enter on ANSI layouts (QWERTY \\).
    Semicolon,      ///< Key right of L (QWERTY ;).
    Apostrophe,     ///< Key right of Semicolon (QWERTY ').
    Grave,          ///< Key left of the number row's 1 (QWERTY `).
    Comma,          ///< Key right of M (QWERTY ,).
    Period,         ///< Key right of Comma (QWERTY .).
    Slash,          ///< Key right of Period (QWERTY /).
    IntlBackslash,  ///< ISO key between left Shift and Z (AZERTY <).
    CapsLock,       ///< Caps Lock.
    PrintScreen,    ///< Print Screen.
    ScrollLock,     ///< Scroll Lock.
    Pause,          ///< Pause/Break.
    Insert,         ///< Insert.
    Delete,         ///< Forward Delete.
    Home,           ///< Home.
    End,            ///< End.
    PageUp,         ///< Page Up.
    PageDown,       ///< Page Down.
    Left,           ///< Left arrow.
    Right,          ///< Right arrow.
    Up,             ///< Up arrow.
    Down,           ///< Down arrow.
    LeftShift,      ///< Left Shift.
    RightShift,     ///< Right Shift.
    LeftCtrl,       ///< Left Control.
    RightCtrl,      ///< Right Control.
    LeftAlt,        ///< Left Alt (Option on macOS).
    RightAlt,       ///< Right Alt (AltGr, Option).
    LeftSuper,      ///< Left Windows/Command key.
    RightSuper,     ///< Right Windows/Command key.
    Keypad0,        ///< Keypad 0.
    Keypad1,        ///< Keypad 1.
    Keypad2,        ///< Keypad 2.
    Keypad3,        ///< Keypad 3.
    Keypad4,        ///< Keypad 4.
    Keypad5,        ///< Keypad 5.
    Keypad6,        ///< Keypad 6.
    Keypad7,        ///< Keypad 7.
    Keypad8,        ///< Keypad 8.
    Keypad9,        ///< Keypad 9.
    KeypadEnter,    ///< Keypad Enter.
    KeypadPlus,     ///< Keypad +.
    KeypadMinus,    ///< Keypad -.
    KeypadMultiply, ///< Keypad *.
    KeypadDivide,   ///< Keypad /.
    KeypadPeriod,   ///< Keypad decimal point.
};

/// Number of Key enumerators, for tables indexed by key.
inline constexpr std::size_t key_count = static_cast<std::size_t>(Key::KeypadPeriod) + 1;

/// A mouse button.
enum class MouseButton : std::uint8_t {
    Left,   ///< Primary button.
    Middle, ///< Wheel button.
    Right,  ///< Secondary button.
    X1,     ///< First side button (usually "back").
    X2,     ///< Second side button (usually "forward").
};

/// Number of MouseButton enumerators, for tables indexed by button.
inline constexpr std::size_t mouse_button_count = static_cast<std::size_t>(MouseButton::X2) + 1;

/// A position or offset in fractional units, as documented at each use.
struct Point {
    float x = 0.0F; ///< Horizontal component, growing rightwards.
    float y = 0.0F; ///< Vertical component, growing downwards.

    /// Memberwise equality.
    friend bool operator==(Point, Point) = default;
};

/// Where the mouse cursor is, in both of a window's coordinate systems.
struct Cursor {
    WindowId window{};    ///< The window the cursor is over.
    Point position;       ///< In window coordinates, like Window::logical_size.
    Point pixel_position; ///< In framebuffer pixels, like Window::pixel_size.

    /// Memberwise equality.
    friend bool operator==(Cursor, Cursor) = default;
};

/// The user or the operating system asked the application to quit.
struct QuitRequested {
    /// Memberwise equality.
    friend bool operator==(QuitRequested, QuitRequested) = default;
};

/// The user asked to close a window (its close button, Alt+F4, Cmd+W).
struct WindowCloseRequested {
    WindowId window{}; ///< The window to close.

    /// Memberwise equality.
    friend bool operator==(WindowCloseRequested, WindowCloseRequested) = default;
};

/// A window's logical or pixel size changed. Carries both current sizes.
struct WindowResized {
    WindowId window{}; ///< The resized window.
    Size logical_size; ///< New size in window coordinates.
    Size pixel_size;   ///< New framebuffer size in pixels.

    /// Memberwise equality.
    friend bool operator==(WindowResized, WindowResized) = default;
};

/// A window's display scale changed, typically because it moved to another
/// display or the user changed the scale setting.
struct WindowScaleChanged {
    WindowId window{};          ///< The affected window.
    float display_scale = 1.0F; ///< New Window::display_scale.

    /// Memberwise equality.
    friend bool operator==(WindowScaleChanged, WindowScaleChanged) = default;
};

/// A key went down. Auto-repeat while it is held is not reported.
struct KeyPressed {
    Key key = Key::Unknown; ///< The physical key.

    /// Memberwise equality.
    friend bool operator==(KeyPressed, KeyPressed) = default;
};

/// A key went up.
struct KeyReleased {
    Key key = Key::Unknown; ///< The physical key.

    /// Memberwise equality.
    friend bool operator==(KeyReleased, KeyReleased) = default;
};

/// A mouse button went down over a window.
struct MouseButtonPressed {
    MouseButton button = MouseButton::Left; ///< The button.
    Cursor cursor;                          ///< Where it was pressed.

    /// Memberwise equality.
    friend bool operator==(MouseButtonPressed, MouseButtonPressed) = default;
};

/// A mouse button went up.
struct MouseButtonReleased {
    MouseButton button = MouseButton::Left; ///< The button.
    Cursor cursor;                          ///< Where it was released.

    /// Memberwise equality.
    friend bool operator==(MouseButtonReleased, MouseButtonReleased) = default;
};

/// The cursor moved within a window.
struct MouseMoved {
    Cursor cursor; ///< The new position.

    /// Memberwise equality.
    friend bool operator==(MouseMoved, MouseMoved) = default;
};

/// The wheel or trackpad scrolled.
struct MouseWheel {
    WindowId window{}; ///< The window under the cursor.
    Point delta; ///< Scroll steps; positive y scrolls away from the user, whatever the system's direction
                 ///< setting.

    /// Memberwise equality.
    friend bool operator==(MouseWheel, MouseWheel) = default;
};

/// The cursor entered a window.
struct CursorEntered {
    WindowId window{}; ///< The window entered.

    /// Memberwise equality.
    friend bool operator==(CursorEntered, CursorEntered) = default;
};

/// The cursor left a window.
struct CursorLeft {
    WindowId window{}; ///< The window left.

    /// Memberwise equality.
    friend bool operator==(CursorLeft, CursorLeft) = default;
};

/// Everything the event pump reports.
using Event = std::variant<QuitRequested, WindowCloseRequested, WindowResized, WindowScaleChanged, KeyPressed,
                           KeyReleased, MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseWheel,
                           CursorEntered, CursorLeft>;

/// Why the platform could not start.
enum class PlatformError : std::uint8_t {
    AlreadyInitialized, ///< Another Platform is alive; there is at most one per process.
    VideoUnavailable,   ///< No usable video driver (no display, or the requested driver is missing).
};

/// The application's connection to the operating system: owns the video
/// subsystem, creates windows and pumps events.
///
/// At most one Platform exists at a time, and it must be used from the main
/// thread. It must outlive every Window it creates. Move-only.
///
/// @code
/// auto platform = opus::Platform::create();
/// auto window = platform->create_window({.title = "Skirmish"});
/// for (const opus::Event& event : platform->poll_events()) { /* ... */ }
/// @endcode
class Platform {
public:
    /// Starts the video subsystem.
    [[nodiscard]] static std::expected<Platform, PlatformError> create();

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    /// Takes over @p other's connection; @p other no longer owns one.
    Platform(Platform&& other) noexcept;
    /// Shuts down the currently owned connection, then takes @p other's.
    Platform& operator=(Platform&& other) noexcept;
    /// Shuts the video subsystem down, destroying any window still open.
    ~Platform();

    /// Creates a window as described by @p config.
    [[nodiscard]] std::expected<Window, WindowError> create_window(const WindowConfig& config) const;

    /// Drains the operating system's event queue.
    ///
    /// Several size changes of one window within a call are reported as a
    /// single WindowResized carrying the final sizes, at the position of the
    /// latest change. The returned events stay valid until the next call.
    [[nodiscard]] std::span<const Event> poll_events();

    /// The label of @p key on the user's current keyboard layout, for showing
    /// bindings: Key::Q is "A" on AZERTY, Key::LeftShift is "Left Shift".
    /// Empty for Key::Unknown, and on a moved-from platform.
    [[nodiscard]] std::string key_label(Key key) const;

private:
    Platform() = default;

    bool owns_video_ = false;
    std::vector<Event> events_;
};

} // namespace opus
