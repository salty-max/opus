#include "paths.hpp"
#include "rules.hpp"
#include "source.hpp"

#include <regex>

namespace opus::lint {

Diagnostics check_pragma_once(const SourceFile& file) {
    Diagnostics out;
    if (!is_header(file.path)) {
        return out;
    }
    static const std::regex pragma_once{R"(^\s*#\s*pragma\s+once\s*$)"};
    for (const SourceLine& line : file.lines) {
        if (std::regex_search(line.code, pragma_once)) {
            return out;
        }
    }
    out.push_back(
        {.path = file.path, .line = 0, .rule = "pragma-once", .message = "header lacks `#pragma once`"});
    return out;
}

} // namespace opus::lint
