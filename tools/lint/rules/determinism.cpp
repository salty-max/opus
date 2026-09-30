#include "paths.hpp"
#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <array>

namespace opus::lint {

Diagnostics check_determinism(const SourceFile& file) {
    Diagnostics out;
    if (!is_deterministic_module(file.path)) {
        return out;
    }
    using detail::pattern;
    static const std::array code_patterns{
        pattern(R"(\b(float|double)\b)", "floating point in a simulation module; use opus fixed-point types"),
        pattern(
            R"(std::(sin|cos|tan|asin|acos|atan2?|sqrt|cbrt|hypot|pow|exp|log|floor|ceil|round|fmod)\s*\()",
            "libm call in a simulation module; use opus fixed-point math"),
        pattern(R"(std::unordered_\w+)", "unordered container in a simulation module; its iteration order "
                                         "differs across standard libraries"),
        pattern(R"(std::chrono|(^|[^\w.>:])(time|clock)\s*\()",
                "clock read in a simulation module; simulation time is the tick counter"),
        pattern(
            R"((^|[^\w.>:])s?rand\s*\(|std::(random_device|mt19937\w*|minstd_rand\w*|default_random_engine|\w+_distribution)\b)",
            "platform RNG in a simulation module; use the seeded simulation RNG"),
        pattern(R"(std::(ranges::)?(sort|partial_sort|nth_element)\b)",
                "unstable sort in a simulation module; equal elements land differently across standard "
                "libraries"),
    };
    static const std::array include_patterns{
        pattern(R"(^\s*#\s*include\s*<(chrono|random|cmath|math\.h|ctime|unordered_map|unordered_set)>)",
                "non-deterministic facility included in a simulation module"),
    };
    detail::report_matches(file, "determinism", detail::Field::Code, code_patterns, out);
    detail::report_matches(file, "determinism", detail::Field::Raw, include_patterns, out);
    return out;
}

} // namespace opus::lint
