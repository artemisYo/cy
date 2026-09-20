#pragma once

#include "linked_list.hxx"

#include <string_view>

struct File;
struct Struct;
struct Field;
struct Expr;
struct Value;
struct Array;

struct File {
    List<Expr> items;

    void dump(int depth = 0);
};

struct Struct {
    List<Field> fields;

    void dump(int depth = 0);
};

struct Array {
    List<Expr> elements;

    void dump(int depth = 0);
};

struct Expr {
    Value* value;
    Value* arg; // NULLABLE
    List<Value> concat;

    void dump(int depth = 0);
};

struct Value {
    enum {
        kident,
        kstruct,
        karray,
        kexpr,
    } kind;
    union {
        std::string_view ident;
        Struct ustruct;
        Array array;
        Expr expr;
    };

    void dump(int depth = 0);
};

struct Field {
    std::string_view key;
    Expr value;

    void dump(int depth = 0);
};
