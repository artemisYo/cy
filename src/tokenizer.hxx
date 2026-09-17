#pragma once

#include <ints.hxx>
#include <optional>
#include <string_view>

struct Token {
    enum Kind : u32 {
        identifier,
        colon,
        open_brace,
        close_brace,
        open_bracket,
        close_bracket,
        open_paren,
        close_paren
    } kind;
    u32 offset;

    std::string_view str_repr();
    std::string_view str_of(std::string_view source);
    u32 length(std::string_view source);
};

class Tokenizer {
    std::string_view m_input;
    std::optional<Token> m_tok;
    u32 m_offset;

    void skip_ws();
    std::string_view current() const;

  public:
    Tokenizer() : m_input(""), m_offset(0) {}
    Tokenizer(std::string_view input) : m_input(input), m_offset(0) {
        skip_ws();
    }

    std::optional<Token> next();
    std::optional<Token> next_if(Token::Kind);

    std::optional<Token>& peek();
    std::optional<Token>& peek_if(Token::Kind);

    bool eat();
    bool eat_if(Token::Kind);

    bool done() const;
    bool has(Token::Kind);

    std::string_view str_of(Token&);
    std::string line();
};
