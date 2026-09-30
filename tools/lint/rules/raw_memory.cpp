#include "paths.hpp"
#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <array>

namespace opus::lint {

Diagnostics check_raw_memory(const SourceFile& file) {
    Diagnostics out;
    if (!is_engine(file.path)) {
        return out;
    }
    using detail::pattern;
    static const std::array patterns{
        pattern(R"((^|[^\w])new(\s+[\w:]|\s*\())",
                "raw `new`; own memory through containers, std::unique_ptr or an allocator"),
        pattern(R"(\bdelete\s*(\[\s*\])?\s*[\w(*:])",
                "raw `delete`; own memory through containers, std::unique_ptr or an allocator"),
        pattern(R"((^|[^\w.>:])(malloc|calloc|realloc|free)\s*\()",
                "C allocation; own memory through containers, std::unique_ptr or an allocator"),
    };
    detail::report_matches(file, "raw-memory", detail::Field::Code, patterns, out);
    return out;
}

} // namespace opus::lint
