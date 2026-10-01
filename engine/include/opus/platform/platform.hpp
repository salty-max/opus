#pragma once

#include <opus/platform/window.hpp>

#include <cstdint>
#include <expected>
#include <span>
#include <variant>
#include <vector>

namespace opus {

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

/// Everything the event pump reports.
using Event = std::variant<QuitRequested, WindowCloseRequested, WindowResized, WindowScaleChanged>;

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

private:
    Platform() = default;

    bool owns_video_ = false;
    std::vector<Event> events_;
};

} // namespace opus
