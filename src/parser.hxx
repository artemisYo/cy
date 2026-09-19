#pragma once

#include "arena.hxx"
#include "linked_list.hxx"
#include "tokenizer.hxx"

#include <string>
#include <string_view>

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
    std::string_view name;
    List<Field> fields;

    void dump(int depth = 0);
};

struct Array {
    List<Value> elements;

    void dump(int depth = 0);
};

struct Value {
    struct Exts;

    List<Exts> concat;
    Value* args;
    struct Exts {
        enum {
            kident,
            kstruct,
            karray,
            kvalue,
        } kind;
        union {
            std::string_view ident;
            Struct ustruct;
            Array array;
            Value* value;
        };
    } exts;

    void dump(int depth = 0);
};

struct Field {
    std::string_view key;
    Value value;

    void dump(int depth = 0);
};

class Parser {
  protected:
    Tokenizer lex;
    Arena& ar;

  public:
    Parser(std::string_view input, Arena& arena) : lex(input), ar(arena) {}

    std::string diagnostic();

    bool file(File& out);
};
