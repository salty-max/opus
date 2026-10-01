#include "platform/keys_internal.hpp"

#include <opus/platform/platform.hpp>

#include <SDL3/SDL_scancode.h>

#include <array>
#include <cstddef>
#include <utility>

namespace opus::detail {

namespace {

// One row per Key enumerator except Unknown, in enumerator order, so a key's
// row is at index (key - 1).
constexpr std::array<std::pair<Key, SDL_Scancode>, key_count - 1> key_table{{
    {Key::A, SDL_SCANCODE_A},
    {Key::B, SDL_SCANCODE_B},
    {Key::C, SDL_SCANCODE_C},
    {Key::D, SDL_SCANCODE_D},
    {Key::E, SDL_SCANCODE_E},
    {Key::F, SDL_SCANCODE_F},
    {Key::G, SDL_SCANCODE_G},
    {Key::H, SDL_SCANCODE_H},
    {Key::I, SDL_SCANCODE_I},
    {Key::J, SDL_SCANCODE_J},
    {Key::K, SDL_SCANCODE_K},
    {Key::L, SDL_SCANCODE_L},
    {Key::M, SDL_SCANCODE_M},
    {Key::N, SDL_SCANCODE_N},
    {Key::O, SDL_SCANCODE_O},
    {Key::P, SDL_SCANCODE_P},
    {Key::Q, SDL_SCANCODE_Q},
    {Key::R, SDL_SCANCODE_R},
    {Key::S, SDL_SCANCODE_S},
    {Key::T, SDL_SCANCODE_T},
    {Key::U, SDL_SCANCODE_U},
    {Key::V, SDL_SCANCODE_V},
    {Key::W, SDL_SCANCODE_W},
    {Key::X, SDL_SCANCODE_X},
    {Key::Y, SDL_SCANCODE_Y},
    {Key::Z, SDL_SCANCODE_Z},
    {Key::Digit0, SDL_SCANCODE_0},
    {Key::Digit1, SDL_SCANCODE_1},
    {Key::Digit2, SDL_SCANCODE_2},
    {Key::Digit3, SDL_SCANCODE_3},
    {Key::Digit4, SDL_SCANCODE_4},
    {Key::Digit5, SDL_SCANCODE_5},
    {Key::Digit6, SDL_SCANCODE_6},
    {Key::Digit7, SDL_SCANCODE_7},
    {Key::Digit8, SDL_SCANCODE_8},
    {Key::Digit9, SDL_SCANCODE_9},
    {Key::F1, SDL_SCANCODE_F1},
    {Key::F2, SDL_SCANCODE_F2},
    {Key::F3, SDL_SCANCODE_F3},
    {Key::F4, SDL_SCANCODE_F4},
    {Key::F5, SDL_SCANCODE_F5},
    {Key::F6, SDL_SCANCODE_F6},
    {Key::F7, SDL_SCANCODE_F7},
    {Key::F8, SDL_SCANCODE_F8},
    {Key::F9, SDL_SCANCODE_F9},
    {Key::F10, SDL_SCANCODE_F10},
    {Key::F11, SDL_SCANCODE_F11},
    {Key::F12, SDL_SCANCODE_F12},
    {Key::Escape, SDL_SCANCODE_ESCAPE},
    {Key::Enter, SDL_SCANCODE_RETURN},
    {Key::Tab, SDL_SCANCODE_TAB},
    {Key::Backspace, SDL_SCANCODE_BACKSPACE},
    {Key::Space, SDL_SCANCODE_SPACE},
    {Key::Minus, SDL_SCANCODE_MINUS},
    {Key::Equals, SDL_SCANCODE_EQUALS},
    {Key::LeftBracket, SDL_SCANCODE_LEFTBRACKET},
    {Key::RightBracket, SDL_SCANCODE_RIGHTBRACKET},
    {Key::Backslash, SDL_SCANCODE_BACKSLASH},
    {Key::Semicolon, SDL_SCANCODE_SEMICOLON},
    {Key::Apostrophe, SDL_SCANCODE_APOSTROPHE},
    {Key::Grave, SDL_SCANCODE_GRAVE},
    {Key::Comma, SDL_SCANCODE_COMMA},
    {Key::Period, SDL_SCANCODE_PERIOD},
    {Key::Slash, SDL_SCANCODE_SLASH},
    {Key::IntlBackslash, SDL_SCANCODE_NONUSBACKSLASH},
    {Key::CapsLock, SDL_SCANCODE_CAPSLOCK},
    {Key::PrintScreen, SDL_SCANCODE_PRINTSCREEN},
    {Key::ScrollLock, SDL_SCANCODE_SCROLLLOCK},
    {Key::Pause, SDL_SCANCODE_PAUSE},
    {Key::Insert, SDL_SCANCODE_INSERT},
    {Key::Delete, SDL_SCANCODE_DELETE},
    {Key::Home, SDL_SCANCODE_HOME},
    {Key::End, SDL_SCANCODE_END},
    {Key::PageUp, SDL_SCANCODE_PAGEUP},
    {Key::PageDown, SDL_SCANCODE_PAGEDOWN},
    {Key::Left, SDL_SCANCODE_LEFT},
    {Key::Right, SDL_SCANCODE_RIGHT},
    {Key::Up, SDL_SCANCODE_UP},
    {Key::Down, SDL_SCANCODE_DOWN},
    {Key::LeftShift, SDL_SCANCODE_LSHIFT},
    {Key::RightShift, SDL_SCANCODE_RSHIFT},
    {Key::LeftCtrl, SDL_SCANCODE_LCTRL},
    {Key::RightCtrl, SDL_SCANCODE_RCTRL},
    {Key::LeftAlt, SDL_SCANCODE_LALT},
    {Key::RightAlt, SDL_SCANCODE_RALT},
    {Key::LeftSuper, SDL_SCANCODE_LGUI},
    {Key::RightSuper, SDL_SCANCODE_RGUI},
    {Key::Keypad0, SDL_SCANCODE_KP_0},
    {Key::Keypad1, SDL_SCANCODE_KP_1},
    {Key::Keypad2, SDL_SCANCODE_KP_2},
    {Key::Keypad3, SDL_SCANCODE_KP_3},
    {Key::Keypad4, SDL_SCANCODE_KP_4},
    {Key::Keypad5, SDL_SCANCODE_KP_5},
    {Key::Keypad6, SDL_SCANCODE_KP_6},
    {Key::Keypad7, SDL_SCANCODE_KP_7},
    {Key::Keypad8, SDL_SCANCODE_KP_8},
    {Key::Keypad9, SDL_SCANCODE_KP_9},
    {Key::KeypadEnter, SDL_SCANCODE_KP_ENTER},
    {Key::KeypadPlus, SDL_SCANCODE_KP_PLUS},
    {Key::KeypadMinus, SDL_SCANCODE_KP_MINUS},
    {Key::KeypadMultiply, SDL_SCANCODE_KP_MULTIPLY},
    {Key::KeypadDivide, SDL_SCANCODE_KP_DIVIDE},
    {Key::KeypadPeriod, SDL_SCANCODE_KP_PERIOD},
}};

constexpr bool table_matches_enum() {
    for (std::size_t i = 0; i < key_table.size(); ++i) {
        if (static_cast<std::size_t>(key_table[i].first) != i + 1) {
            return false;
        }
    }
    return true;
}
static_assert(table_matches_enum(), "key_table rows must follow Key's enumerator order");

} // namespace

Key key_from_scancode(SDL_Scancode scancode) {
    for (const auto& [key, code] : key_table) {
        if (code == scancode) {
            return key;
        }
    }
    return Key::Unknown;
}

SDL_Scancode scancode_from_key(Key key) {
    const auto index = static_cast<std::size_t>(key);
    return index == 0 || index > key_table.size() ? SDL_SCANCODE_UNKNOWN : key_table[index - 1].second;
}

} // namespace opus::detail
