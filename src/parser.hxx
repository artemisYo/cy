#pragma once

#include "arena.hxx"
#include "linked_list.hxx"
#include "tokenizer.hxx"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

struct File;
struct Struct;
struct Field;
struct Value;
struct Array;

struct File {
    List<Struct> items;

    void dump(int depth = 0);
};

struct Struct {
    std::optional<std::string_view> name;
    List<Field> fields;

    void dump(int depth = 0);
};

struct Array {
    List<Value> elements;

    void dump(int depth = 0);
};

struct Value {
    using Exts = std::variant<
        std::string_view,
        Struct,
        Array,
        Value*>;
    List<Exts> concat;
    Value* args;
    Exts exts;

    void dump(int depth = 0);
};

struct Field {
    using Vals = std::variant<
        Field*,
        Value>;
    std::string_view key;
    Vals value;

    void dump(int depth = 0);
};

class Parser {
    Tokenizer lex;
    Arena& ar;

  public:
    Parser(std::string_view input, Arena& arena) : lex(input), ar(arena) {}

    std::string diagnostic();

    std::optional<File> pfile();
    std::optional<Struct> pstruct();
    std::optional<Field> pfield();
    std::optional<Array> parray();
    std::optional<Value> pparens();
    std::optional<Value> pvalue();
    std::optional<Value::Exts> ppure_value();
};
