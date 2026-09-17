
#include "tokenizer.hxx"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>

static constexpr std::string_view specials = ":{}[]()";
static constexpr std::string_view whitespace = " \t\n\f\r\v";
static constexpr std::string word_breaks = std::string{specials}
                                               .append(whitespace);

static u32 ws_count(std::string_view s) {
    size_t i = s.find_first_not_of(whitespace);
    if (i == std::string_view::npos) i = s.size();
    return i;
}

static std::optional<std::tuple<Token::Kind, u32>> once(std::string_view input) {
    u32 length = 1;
    Token::Kind kind = Token::identifier;

    if (input.size() == 0 || ws_count(input) != 0) {
        return std::nullopt;
    }

    switch (input.at(0)) {
        case ':':
            kind = Token::colon;
            break;
        case '{':
            kind = Token::open_brace;
            break;
        case '}':
            kind = Token::close_brace;
            break;
        case '[':
            kind = Token::open_bracket;
            break;
        case ']':
            kind = Token::close_bracket;
            break;
        case '(':
            kind = Token::open_paren;
            break;
        case ')':
            kind = Token::close_paren;
            break;
        default: {
            kind = Token::identifier;
            auto idx = input.find_first_of(word_breaks);
            if (idx == std::string_view::npos) {
                length = input.size();
            } else {
                length = idx;
            }
            break;
        }
    }
    return std::tuple{kind, length};
}

std::string_view Token::str_repr() {
    switch (kind) {
        case identifier: return "id";
        case colon: return ":";
        case open_brace: return "{";
        case close_brace: return "}";
        case open_bracket: return "[";
        case close_bracket: return "]";
        case open_paren: return "(";
        case close_paren: return ")";
    }
}

std::string_view Token::str_of(std::string_view source) {
    return source.substr(offset, length(source));
}

u32 Token::length(std::string_view source) {
    switch (kind) {
        case colon:
        case open_brace:
        case close_brace:
        case open_bracket:
        case close_bracket:
        case open_paren:
        case close_paren: return 1;

        case identifier: {
            auto res = once(source.substr(offset));
            assert(res.has_value());
            return std::get<1>(*res);
        }
    }
}

std::string_view Tokenizer::current() const {
    return m_input.substr(m_offset);
}

void Tokenizer::skip_ws() {
    m_offset += ws_count(current());
    while (current().starts_with("--")) {
        auto i = current().find_first_of('\n');
        if (i == std::string_view::npos) {
            i = current().size();
        }
        m_offset += i;
        m_offset += ws_count(current());
    }
}

std::optional<Token> Tokenizer::next() {
    std::optional<Token> out = std::nullopt;
    peek().swap(out);
    return out;
}

std::optional<Token> Tokenizer::next_if(Token::Kind k) {
    std::optional<Token> out = std::nullopt;
    peek_if(k).swap(out);
    return out;
}

std::optional<Token>& Tokenizer::peek() {
    if (m_tok) return m_tok;
    auto res = once(m_input.substr(m_offset));
    if (!res) return m_tok;
    m_tok = Token{
        .kind = std::get<0>(*res),
        .offset = m_offset
    };
    m_offset += std::get<1>(*res);
    skip_ws();
    return m_tok;
}

std::optional<Token>& Tokenizer::peek_if(Token::Kind k) {
    static std::optional<Token> none = std::nullopt;
    auto& t = peek();
    if (t && t->kind == k) return t;
    return none;
}

bool Tokenizer::eat() {
    return next().has_value();
}

bool Tokenizer::eat_if(Token::Kind k) {
    return next_if(k).has_value();
}

bool Tokenizer::done() const {
    return m_input.size() == m_offset;
}

bool Tokenizer::has(Token::Kind k) {
    return peek_if(k).has_value();
}

std::string_view Tokenizer::str_of(Token& t) {
    return t.str_of(m_input);
}

std::string Tokenizer::line() {
    auto seen = m_input.substr(0, m_offset);
    auto idx = seen.rfind('\n');
    if (idx == std::string_view::npos) {
        idx = 0;
    }
    auto line = m_input.substr(idx + 1);
    idx = line.find('\n');
    if (idx != std::string_view::npos) {
        idx++;
    }
    line = line.substr(0, idx);
    return std::string(line);
}
