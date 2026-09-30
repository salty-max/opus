#include "diagnostic.hpp"
#include "rules.hpp"
#include "source.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace opus::lint {

namespace {

using FileRule = Diagnostics (*)(const SourceFile&);

constexpr std::array file_rules{
    &check_comments,    &check_casts,      &check_includes,     &check_engine_io,   &check_raw_memory,
    &check_determinism, &check_test_names, &check_suppressions, &check_pragma_once,
};

} // namespace

Diagnostics lint_file(const SourceFile& file) {
    Diagnostics out;
    for (const FileRule rule : file_rules) {
        for (Diagnostic& diagnostic : rule(file)) {
            // A malformed suppression cannot suppress its own report.
            const bool silenced = diagnostic.rule != "suppression" && diagnostic.line > 0 &&
                                  has_marker_above(file, diagnostic.line - 1, "allow-strict:");
            if (!silenced) {
                out.push_back(std::move(diagnostic));
            }
        }
    }
    std::ranges::stable_sort(out, {}, &Diagnostic::line);
    return out;
}

} // namespace opus::lint
