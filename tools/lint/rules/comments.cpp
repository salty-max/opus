#include "rules.hpp"
#include "rules/internal.hpp"
#include "source.hpp"

#include <array>

namespace opus::lint {

Diagnostics check_comments(const SourceFile& file) {
    using detail::pattern;
    static const std::array patterns{
        pattern(R"((^|[^\w&])#[0-9]+\b)",
                "issue reference in a comment; issue numbers belong in commits, PRs and changesets"),
        pattern(R"(\bv[0-9]+\.[0-9]+)", "version marker in a comment; code describes what works today"),
        pattern(R"(\bM[0-9]{1,2}\b)", "milestone marker in a comment; code describes what works today"),
        pattern(R"(\b(phase\s+[0-9]+|roadmap:))",
                "roadmap marker in a comment; code describes what works today", true),
        pattern(
            R"(\b(previously|was a bug|now fixed|surfaced while|regression:|used to (be|have|return|take|call|work|need|require)\b))",
            "change-history narration in a comment; state the invariant, the story goes in the commit", true),
        pattern(
            R"(\b(claude|anthropic|chatgpt|copilot|openai)\b|generated with|co-authored-by|ai[- ]generated|\xF0\x9F\xA4\x96)",
            "AI attribution in a comment", true),
    };
    Diagnostics out;
    detail::report_matches(file, "comment-content", detail::Field::Comment, patterns, out);
    return out;
}

} // namespace opus::lint
