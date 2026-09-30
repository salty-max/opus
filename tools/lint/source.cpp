#include "source.hpp"

#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace opus::lint {

namespace {

enum class State : unsigned char { Code, BlockComment, String, Char, RawString };

bool is_word_char(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

// A quote starts a raw string when the identifier directly before it is
// one of the raw prefixes (R, LR, uR, UR, u8R).
bool is_raw_prefix(std::string_view line, std::size_t quote) {
    std::size_t start = quote;
    while (start > 0 && is_word_char(line[start - 1])) {
        --start;
    }
    const std::string_view prefix = line.substr(start, quote - start);
    return prefix == "R" || prefix == "LR" || prefix == "uR" || prefix == "UR" || prefix == "u8R";
}

// `1'000` uses the quote as a digit separator, not a char literal.
bool is_digit_separator(std::string_view line, std::size_t quote) {
    return quote > 0 && quote + 1 < line.size() &&
           std::isxdigit(static_cast<unsigned char>(line[quote - 1])) != 0 &&
           std::isxdigit(static_cast<unsigned char>(line[quote + 1])) != 0;
}

// Doc-comment markers (`///`, `//!`, `/**`, `///<`) are syntax, not prose.
std::string_view strip_doc_marker(std::string_view comment) {
    while (!comment.empty() && (comment.front() == '/' || comment.front() == '!' || comment.front() == '*' ||
                                comment.front() == '<')) {
        comment.remove_prefix(1);
    }
    return comment;
}

void append_comment(SourceLine& line, std::string_view text) {
    const std::string_view cleaned = trim(strip_doc_marker(text));
    if (cleaned.empty()) {
        return;
    }
    if (!line.comment.empty()) {
        line.comment += ' ';
    }
    line.comment += cleaned;
}

class Splitter {
public:
    void feed_line(std::string_view text) {
        SourceLine line{.raw = std::string{text}, .code = {}, .comment = {}};
        std::size_t comment_start = 0;
        for (std::size_t i = 0; i < text.size(); ++i) {
            const char c = text[i];
            const char next = i + 1 < text.size() ? text[i + 1] : '\0';
            switch (state_) {
            case State::Code:
                if (c == '/' && next == '/') {
                    append_comment(line, text.substr(i + 2));
                    i = text.size();
                } else if (c == '/' && next == '*') {
                    state_ = State::BlockComment;
                    comment_start = i + 2;
                    ++i;
                } else if (c == '"' && is_raw_prefix(text, i)) {
                    const std::size_t open = text.find('(', i + 1);
                    const std::size_t delimiter_end = open == std::string_view::npos ? text.size() : open;
                    raw_terminator_ = ")" + std::string{text.substr(i + 1, delimiter_end - i - 1)} + "\"";
                    state_ = State::RawString;
                    line.code += '"';
                    i = delimiter_end;
                } else if (c == '"') {
                    state_ = State::String;
                    line.code += c;
                } else if (c == '\'' && !is_digit_separator(text, i)) {
                    state_ = State::Char;
                    line.code += c;
                } else {
                    line.code += c;
                }
                break;
            case State::BlockComment:
                if (c == '*' && next == '/') {
                    append_comment(line, text.substr(comment_start, i - comment_start));
                    state_ = State::Code;
                    ++i;
                }
                break;
            case State::String:
            case State::Char:
                if (c == '\\') {
                    ++i;
                } else if ((state_ == State::String && c == '"') || (state_ == State::Char && c == '\'')) {
                    line.code += c;
                    state_ = State::Code;
                }
                break;
            case State::RawString:
                if (text.substr(i).starts_with(raw_terminator_)) {
                    line.code += '"';
                    i += raw_terminator_.size() - 1;
                    state_ = State::Code;
                }
                break;
            }
        }
        if (state_ == State::BlockComment) {
            append_comment(line, text.substr(comment_start));
        }
        // Ordinary literals cannot span lines; an unterminated one ends here.
        if (state_ == State::String || state_ == State::Char) {
            state_ = State::Code;
        }
        lines_.push_back(std::move(line));
    }

    std::vector<SourceLine> take() { return std::move(lines_); }

private:
    State state_ = State::Code;
    std::string raw_terminator_;
    std::vector<SourceLine> lines_;
};

} // namespace

std::string_view trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}

SourceFile parse_source(std::string path, std::string_view text) {
    Splitter splitter;
    while (!text.empty()) {
        const std::size_t newline = text.find('\n');
        std::string_view line = text.substr(0, newline);
        if (line.ends_with('\r')) {
            line.remove_suffix(1);
        }
        splitter.feed_line(line);
        text.remove_prefix(newline == std::string_view::npos ? text.size() : newline + 1);
    }
    return {.path = std::move(path), .lines = splitter.take()};
}

bool has_marker_above(const SourceFile& file, std::size_t index, std::string_view marker) {
    for (std::size_t i = index; i > 0; --i) {
        const SourceLine& line = file.lines[i - 1];
        if (!trim(line.code).empty() || line.comment.empty()) {
            return false;
        }
        const std::string_view comment = trim(line.comment);
        if (comment.starts_with(marker)) {
            return !trim(comment.substr(marker.size())).empty();
        }
    }
    return false;
}

} // namespace opus::lint
