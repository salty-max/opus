#pragma once

#include "rules.hpp"
#include "source.hpp"

#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace opus::lint::test {

/// A per-file rule, as declared in rules.hpp.
using FileRule = Diagnostics (*)(const SourceFile&);

/// Runs @p rule over @p text as if it lived at @p path, returning each
/// finding as `"<line>:<rule>"` so tests compare locations, not wording.
inline std::vector<std::string> findings(FileRule rule, std::string path, std::string_view text) {
    std::vector<std::string> out;
    for (const Diagnostic& diagnostic : rule(parse_source(std::move(path), text))) {
        out.push_back(std::format("{}:{}", diagnostic.line, diagnostic.rule));
    }
    return out;
}

/// Shorthand for the empty findings list.
inline const std::vector<std::string> none{};

} // namespace opus::lint::test
