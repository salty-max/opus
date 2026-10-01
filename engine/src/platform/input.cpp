#include <opus/platform/input.hpp>

#include <opus/platform/platform.hpp>
#include <opus/platform/window.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>

namespace opus {

namespace {

std::size_t index_of(Key key) {
    return static_cast<std::size_t>(key);
}

std::size_t index_of(MouseButton button) {
    return static_cast<std::size_t>(button);
}

// A press of something already down (a release was lost, e.g. while the
// window was unfocused) is not a new edge.
template <typename Edges> void press(Edges& edges, std::size_t index) {
    if (!edges.down[index]) {
        edges.pressed[index] = true;
    }
    edges.down[index] = true;
}

template <typename Edges> void release(Edges& edges, std::size_t index) {
    if (edges.down[index]) {
        edges.released[index] = true;
    }
    edges.down[index] = false;
}

} // namespace

void InputState::begin_frame() {
    keys_.pressed.reset();
    keys_.released.reset();
    buttons_.pressed.reset();
    buttons_.released.reset();
    wheel_ = {};
}

void InputState::apply(const Event& event) {
    if (const auto* key = std::get_if<KeyPressed>(&event)) {
        press(keys_, index_of(key->key));
    } else if (const auto* key_up = std::get_if<KeyReleased>(&event)) {
        release(keys_, index_of(key_up->key));
    } else if (const auto* button = std::get_if<MouseButtonPressed>(&event)) {
        press(buttons_, index_of(button->button));
        cursor_ = button->cursor;
    } else if (const auto* button_up = std::get_if<MouseButtonReleased>(&event)) {
        release(buttons_, index_of(button_up->button));
        cursor_ = button_up->cursor;
    } else if (const auto* moved = std::get_if<MouseMoved>(&event)) {
        cursor_ = moved->cursor;
    } else if (const auto* wheel = std::get_if<MouseWheel>(&event)) {
        wheel_.x += wheel->delta.x;
        wheel_.y += wheel->delta.y;
    } else if (const auto* left = std::get_if<CursorLeft>(&event)) {
        if (cursor_ && cursor_->window == left->window) {
            cursor_.reset();
        }
    }
}

bool InputState::down(Key key) const {
    return keys_.down[index_of(key)];
}

bool InputState::pressed(Key key) const {
    return keys_.pressed[index_of(key)];
}

bool InputState::released(Key key) const {
    return keys_.released[index_of(key)];
}

bool InputState::down(MouseButton button) const {
    return buttons_.down[index_of(button)];
}

bool InputState::pressed(MouseButton button) const {
    return buttons_.pressed[index_of(button)];
}

bool InputState::released(MouseButton button) const {
    return buttons_.released[index_of(button)];
}

Modifiers InputState::modifiers() const {
    return {
        .shift = down(Key::LeftShift) || down(Key::RightShift),
        .ctrl = down(Key::LeftCtrl) || down(Key::RightCtrl),
        .alt = down(Key::LeftAlt) || down(Key::RightAlt),
        .super = down(Key::LeftSuper) || down(Key::RightSuper),
    };
}

std::optional<Cursor> InputState::cursor() const {
    return cursor_;
}

Point InputState::wheel() const {
    return wheel_;
}

EdgeScroll edge_scroll(Point position, Size window_size, float margin) {
    const auto width = static_cast<float>(window_size.width);
    const auto height = static_cast<float>(window_size.height);
    const bool inside = position.x >= 0.0F && position.y >= 0.0F && position.x < width && position.y < height;
    if (margin <= 0.0F || !inside) {
        return {};
    }
    const auto axis = [margin](float coordinate, float extent) -> std::int8_t {
        if (coordinate < margin) {
            return -1;
        }
        return coordinate >= extent - margin ? std::int8_t{1} : std::int8_t{0};
    };
    return {.x = axis(position.x, width), .y = axis(position.y, height)};
}

} // namespace opus
