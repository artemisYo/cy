#pragma once

#include "tokenizer.hxx"

#include <memory>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

struct File;
struct Struct;
struct Field;
struct Value;
struct Array;

struct File {
    std::vector<Struct> items;

    void dump(int depth = 0);
};

struct Struct {
    std::optional<std::string_view> name;
    std::vector<Field> fields;

    void dump(int depth = 0);
};

struct Array {
    std::vector<Value> elements;

    void dump(int depth = 0);
};

struct Value {
    using Exts = std::variant<
        std::string_view,
        Struct,
        Array,
        std::unique_ptr<Value>>;
    std::vector<Exts> concat;
    std::unique_ptr<Value> args;
    Exts exts;

    void dump(int depth = 0);
};

struct Field {
    using Vals = std::variant<
        std::unique_ptr<Field>,
        Value>;
    std::string_view key;
    Vals value;

    void dump(int depth = 0);
};

class Parser {
    Tokenizer lex;

  public:
    Parser(std::string_view input) : lex(input) {}

    std::string diagnostic();

    std::optional<File> pfile();
    std::optional<Struct> pstruct();
    std::optional<Field> pfield();
    std::optional<Array> parray();
    std::optional<Value> pparens();
    std::optional<Value> pvalue();
    std::optional<Value::Exts> ppure_value();
};
