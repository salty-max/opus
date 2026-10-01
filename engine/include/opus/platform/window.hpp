#pragma once

#include <cstdint>
#include <string>

namespace opus {

/// Identifies a window for its whole life; events name windows by id.
/// The zero value never names a live window.
enum class WindowId : std::uint32_t {};

/// A width and height in whole units (points or pixels, as documented at
/// each use).
struct Size {
    std::int32_t width = 0;  ///< Horizontal extent.
    std::int32_t height = 0; ///< Vertical extent.

    /// Memberwise equality.
    friend bool operator==(Size, Size) = default;
};

/// How to create a window.
struct WindowConfig {
    /// Logical size used when none is given: 1280×720, a 16:9 size that fits most desktop displays.
    static constexpr Size default_size{.width = 1280, .height = 720};

    std::string title = "Opus"; ///< Title bar text, UTF-8.
    Size size = default_size;   ///< Logical size; both extents must be positive.
    bool resizable = true;      ///< Whether the user may resize the window.
    bool fullscreen = false;    ///< Fullscreen on the window's display instead of windowed.
};

/// Why a window could not be created.
enum class WindowError : std::uint8_t {
    InvalidSize,      ///< WindowConfig::size has a zero or negative extent.
    PlatformInactive, ///< The Platform was moved from and no longer owns the video subsystem.
    CreationFailed,   ///< The operating system refused to create the window.
};

class Platform;

/// An operating-system window with high-DPI support.
///
/// Created by Platform::create_window and destroyed with this object. Move-only.
/// The Platform that created a window must outlive it; a window outliving its
/// platform reports zero sizes and is safe to destroy.
///
/// Sizes come in two units (docs/platform.md, "Sizes"): the **logical size**
/// is in the window coordinates that mouse positions use, and the **pixel
/// size** is the drawable framebuffer the renderer targets.
class Window {
public:
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    /// Takes ownership of @p other's window; @p other no longer names one.
    Window(Window&& other) noexcept;
    /// Destroys the currently owned window, then takes @p other's.
    Window& operator=(Window&& other) noexcept;
    ~Window();

    /// This window's id, as carried by its events; the zero id once moved from.
    [[nodiscard]] WindowId id() const;

    /// Size in window coordinates (points on macOS, pixels on most other systems).
    [[nodiscard]] Size logical_size() const;

    /// Size of the drawable framebuffer in pixels.
    [[nodiscard]] Size pixel_size() const;

    /// Factor to scale UI drawn in pixel space by, so it matches the user's
    /// display scale setting (2.0 on a Retina display, 1.5 at 150% on Windows).
    [[nodiscard]] float display_scale() const;

private:
    friend class Platform;
    explicit Window(WindowId id);

    WindowId id_{};
};

} // namespace opus
