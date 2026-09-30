#include "diagnostic.hpp"

#include <format>
#include <string>

namespace opus::lint {

std::string format(const Diagnostic& diagnostic) {
    return std::format("{}:{}: error[{}]: {}", diagnostic.path, diagnostic.line, diagnostic.rule,
                       diagnostic.message);
}

} // namespace opus::lint
