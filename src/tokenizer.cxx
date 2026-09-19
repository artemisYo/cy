
#include "tokenizer.hxx"

#include <cassert>
#include <string_view>

static constexpr std::string_view whitespace = " \t\n\f\r\v";
static constexpr std::string_view word_breaks = ":{}[]() \t\n\f\r\v";

static u32 ws_count(std::string_view s) {
    size_t i = s.find_first_not_of(whitespace);
    if (i == std::string_view::npos) i = s.size();
    return i;
}

static bool once(
    std::string_view input,
    Token::Kind& out_k,
    u32& out_l
) {
    out_l = 1;

    if (input.size() == 0 || ws_count(input) != 0) {
        return false;
    }

    switch (input.at(0)) {
        case ':':
            out_k = Token::colon;
            break;
        case '{':
            out_k = Token::open_brace;
            break;
        case '}':
            out_k = Token::close_brace;
            break;
        case '[':
            out_k = Token::open_bracket;
            break;
        case ']':
            out_k = Token::close_bracket;
            break;
        case '(':
            out_k = Token::open_paren;
            break;
        case ')':
            out_k = Token::close_paren;
            break;
        default: {
            out_k = Token::identifier;
            auto idx = input.find_first_of(word_breaks);
            if (idx == std::string_view::npos) {
                out_l = input.size();
            } else {
                out_l = idx;
            }
            break;
        }
    }
    return true;
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
            Token::Kind k;
            u32 l;
            assert(once(source.substr(offset), k, l));
            return l;
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

bool Tokenizer::next(Token& out) {
    if (!peek(out)) return false;
    has_token = false;
    return true;
}

bool Tokenizer::next_if(Token::Kind k, Token& out) {
    if (!peek_if(k, out)) return false;
    has_token = false;
    return true;
}

bool Tokenizer::peek(Token& out) {
    if (!has_token) {
        Token::Kind k;
        u32 l;
        if (!once(m_input.substr(m_offset), k, l)) return false;
        m_tok = Token{
            .kind = k,
            .offset = m_offset
        };
        m_offset += l;
        has_token = true;
        skip_ws();
    }
    out = m_tok;
    return true;
}

bool Tokenizer::peek_if(Token::Kind k, Token& out) {
    if (peek(out) && out.kind == k) return true;
    return false;
}

bool Tokenizer::eat() {
    Token ignore{};
    return next(ignore);
}

bool Tokenizer::eat_if(Token::Kind k) {
    Token ignore{};
    return next_if(k, ignore);
}

bool Tokenizer::done() const {
    return m_input.size() == m_offset;
}

bool Tokenizer::has(Token::Kind k) {
    Token ignore{};
    return peek_if(k, ignore);
}

std::string_view Tokenizer::str_of(Token& t) {
    return t.str_of(m_input);
}

std::string_view Tokenizer::line() {
    auto seen = m_input.substr(0, m_offset);
    auto idx = seen.rfind('\n');
    if (idx == std::string_view::npos) {
        idx = 0;
    } else {
        idx += 1;
    }
    auto line = m_input.substr(idx);
    idx = line.find('\n');
    if (idx != std::string_view::npos) {
        idx++;
    }
    return line.substr(0, idx);
}
