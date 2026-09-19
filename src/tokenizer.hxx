#pragma once

#include <ints.hxx>
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
    Token m_tok{};
    bool has_token = false;
    u32 m_offset = 0;

    void skip_ws();
    std::string_view current() const;

  public:
    Tokenizer() : m_input("") {}
    Tokenizer(std::string_view input) : m_input(input) {
        skip_ws();
    }

    bool next(Token&);
    bool next_if(Token::Kind, Token&);

    bool peek(Token&);
    bool peek_if(Token::Kind, Token&);

    bool eat();
    bool eat_if(Token::Kind);

    bool done() const;
    bool has(Token::Kind);

    std::string_view str_of(Token&);
    std::string_view line();
};
