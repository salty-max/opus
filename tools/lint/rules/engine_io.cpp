#include "paths.hpp"
#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <array>

namespace opus::lint {

Diagnostics check_engine_io(const SourceFile& file) {
    Diagnostics out;
    if (!is_engine(file.path)) {
        return out;
    }
    using detail::pattern;
    static const std::array code_patterns{
        pattern(
            R"(std::(cout|cerr|clog|print|println|printf|fprintf|puts|fputs|putchar)\b|(^|[^\w.>:])(printf|fprintf|puts|fputs|putchar)\s*\(|\b(stdout|stderr)\b)",
            "direct console output in the engine; write through an opus::Logger"),
    };
    static const std::array include_patterns{
        pattern(R"(^\s*#\s*include\s*<(iostream|print)>)",
                "console-stream header in the engine; write through an opus::Logger"),
    };
    detail::report_matches(file, "engine-io", detail::Field::Code, code_patterns, out);
    detail::report_matches(file, "engine-io", detail::Field::Raw, include_patterns, out);
    return out;
}

} // namespace opus::lint
