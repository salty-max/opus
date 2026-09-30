#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <cstddef>
#include <regex>

namespace opus::lint {

Diagnostics check_casts(const SourceFile& file) {
    static const std::regex unsafe_cast{R"(\b(reinterpret_cast|const_cast|std::bit_cast|std::launder)\b)"};
    Diagnostics out;
    for (std::size_t i = 0; i < file.lines.size(); ++i) {
        if (std::regex_search(file.lines[i].code, unsafe_cast) && !has_marker_above(file, i, "safety:")) {
            detail::report(file, i, "cast-safety",
                           "type-punning cast needs a `// safety: <reason>` comment directly above", out);
        }
    }
    return out;
}

} // namespace opus::lint
