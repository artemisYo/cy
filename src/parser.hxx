#pragma once

#include "arena.hxx"
#include "ast.hxx"
#include "tokenizer.hxx"

#include <string>
#include <string_view>

class Parser {
  protected:
    Tokenizer lex;
    Arena& ar;

  public:
    Parser(std::string_view input, Arena& arena) : lex(input), ar(arena) {}

    std::string diagnostic();

    bool file(File& out);
};
