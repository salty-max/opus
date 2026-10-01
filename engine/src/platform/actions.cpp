#include <opus/platform/actions.hpp>

#include <opus/platform/input.hpp>
#include <opus/platform/platform.hpp>

#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace opus {

namespace {

enum class Edge : unsigned char { Down, Pressed, Released };

// The modifiers a chord compares against: a modifier key used as the chord's
// own input does not count as an extra modifier.
Modifiers modifiers_for(const Chord& chord, Modifiers held) {
    if (const auto* key = std::get_if<Key>(&chord.input)) {
        switch (*key) {
        case Key::LeftShift:
        case Key::RightShift:
            held.shift = chord.modifiers.shift;
            break;
        case Key::LeftCtrl:
        case Key::RightCtrl:
            held.ctrl = chord.modifiers.ctrl;
            break;
        case Key::LeftAlt:
        case Key::RightAlt:
            held.alt = chord.modifiers.alt;
            break;
        case Key::LeftSuper:
        case Key::RightSuper:
            held.super = chord.modifiers.super;
            break;
        default:
            break;
        }
    }
    return held;
}

bool has_edge(const InputState& input, const Chord& chord, Edge edge) {
    return std::visit(
        [&input, edge](auto trigger) {
            switch (edge) {
            case Edge::Down:
                return input.down(trigger);
            case Edge::Pressed:
                return input.pressed(trigger);
            case Edge::Released:
                return input.released(trigger);
            }
            return false;
        },
        chord.input);
}

bool any_chord(std::span<const Chord> chords, const InputState& input, Edge edge) {
    return std::ranges::any_of(chords, [&input, edge](const Chord& chord) {
        return has_edge(input, chord, edge) && modifiers_for(chord, input.modifiers()) == chord.modifiers;
    });
}

} // namespace

void ActionMap::bind(std::string_view action, const Chord& chord) {
    auto found = bindings_.find(action);
    if (found == bindings_.end()) {
        found = bindings_.emplace(std::string{action}, std::vector<Chord>{}).first;
    }
    if (std::ranges::find(found->second, chord) == found->second.end()) {
        found->second.push_back(chord);
    }
}

void ActionMap::unbind(std::string_view action) {
    if (const auto found = bindings_.find(action); found != bindings_.end()) {
        bindings_.erase(found);
    }
}

std::span<const Chord> ActionMap::chords(std::string_view action) const {
    const auto found = bindings_.find(action);
    return found == bindings_.end() ? std::span<const Chord>{} : std::span<const Chord>{found->second};
}

bool ActionMap::held(const InputState& input, std::string_view action) const {
    return any_chord(chords(action), input, Edge::Down);
}

bool ActionMap::triggered(const InputState& input, std::string_view action) const {
    return any_chord(chords(action), input, Edge::Pressed);
}

bool ActionMap::ended(const InputState& input, std::string_view action) const {
    return any_chord(chords(action), input, Edge::Released);
}

} // namespace opus
