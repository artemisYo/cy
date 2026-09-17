#include "parser.hxx"

#include <cstdio>
#include <optional>

std::optional<File> Parser::pfile() {
    File out{};
    auto res = pstruct();
    if (!res) return std::nullopt;
    out.items.push_back(std::move(*res));
    while (!lex.done()) {
        res = pstruct();
        if (!res) return std::nullopt;
        out.items.push_back(std::move(*res));
    }
    return out;
}

std::optional<Struct> Parser::pstruct() {
    Struct out{};

    out.name = lex.next_if(Token::identifier)
                   .transform([&](auto t) { return lex.str_of(t); });

    if (!lex.eat_if(Token::open_brace)) return std::nullopt;

    auto res = pfield();
    if (!res) return std::nullopt;
    out.fields.push_back(std::move(*res));

    while (!lex.eat_if(Token::close_brace)) {
        res = pfield();
        if (!res) return std::nullopt;
        out.fields.push_back(std::move(*res));
    }

    return out;
}

std::optional<Field> Parser::pfield() {
    Field out{};

    auto t = lex.next_if(Token::identifier);
    if (!t) return std::nullopt;
    out.key = lex.str_of(*t);

    if (!lex.next_if(Token::colon)) return std::nullopt;

    auto lookahead = lex;
    lookahead.next();
    if (lookahead.eat_if(Token::colon)) {
        auto res = pfield();
        if (!res) return std::nullopt;
        auto value = new Field{std::move(*res)};
        out.value = std::unique_ptr<Field>{value};
    } else {
        auto res = pvalue();
        if (!res) return std::nullopt;
        out.value = std::move(*res);
    }

    return out;
}

std::optional<Array> Parser::parray() {
    Array out{};

    if (!lex.eat_if(Token::open_bracket)) return std::nullopt;
    while (!lex.eat_if(Token::close_bracket)) {
        auto res = pvalue();
        if (!res) return std::nullopt;
        out.elements.push_back(std::move(*res));
    }

    return out;
}

std::optional<Value> Parser::pparens() {
    if (!lex.eat_if(Token::open_paren)) return std::nullopt;
    auto res = pvalue();
    if (!res) return std::nullopt;
    if (!lex.eat_if(Token::close_paren)) return std::nullopt;

    return std::move(*res);
}

std::optional<Value> Parser::pvalue() {
    Value out{};

    {
        auto res = ppure_value();
        if (!res) return std::nullopt;
        out.exts = std::move(*res);
    }

    if (lex.has(Token::open_paren)) {
        auto res = pparens();
        if (!res) return std::nullopt;
        auto args = new Value{std::move(*res)};
        out.args = std::unique_ptr<Value>(args);
    } else if (lex.has(Token::open_bracket)) {
        auto res = parray();
        if (!res) return std::nullopt;
        auto args = new Value{};
        args->exts = std::move(*res);
        out.args = std::unique_ptr<Value>(args);
    }

    for (auto t = lex.peek(); t && lex.str_of(*t) == "+"; t = lex.peek()) {
        lex.next()->str_repr();
        auto res = ppure_value();
        if (!res) return std::nullopt;
        out.concat.push_back(std::move(*res));
    }

    return out;
}

std::optional<Value::Exts> Parser::ppure_value() {
    if (lex.done()) return std::nullopt;

    switch (lex.peek()->kind) {
        default: return std::nullopt;
        case Token::identifier: {
            auto t = *lex.next();
            return lex.str_of(t);
        }
        case Token::open_brace: {
            return pstruct();
        }
        case Token::open_bracket: {
            return parray();
        }
        case Token::open_paren: {
            auto res = pparens();
            if (!res) return std::nullopt;
            auto v = new Value{std::move(*res)};
            return std::unique_ptr<Value>{v};
        }
    }
}

std::string Parser::diagnostic() {
    std::string s = "Unexpected token '";
    s.append((*lex.peek()).str_repr())
        .append("'")
        .append("\n")
        .append("Input: ")
        .append(lex.line());
    return s;
}

static void indent(int depth) {
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
}

void File::dump(int depth) {
    for (auto& s : items) {
        indent(depth);
        s.dump(depth + 1);
        printf("\n");
    }
}

void Struct::dump(int depth) {
    if (name) {
        printf("%.*s ", (int)name->size(), name->data());
    }
    printf("{\n");
    for (auto& f : fields) {
        indent(depth);
        f.dump(depth + 1);
        printf("\n");
    }
    indent(depth - 1);
    printf("}");
}

void Field::dump(int depth) {
    printf("%.*s: ", (int)key.size(), key.data());
    struct Visitor {
        int depth;
        Visitor(int d) : depth(d) {}

        void operator()(std::unique_ptr<Field>& f) {
            f->dump(depth);
        }
        void operator()(Value& v) {
            v.dump(depth);
        }
    };
    std::visit(Visitor{depth}, value);
}

void Value::dump(int depth) {
    struct Visitor {
        int depth;
        Visitor(int d) : depth(d) {}

        void operator()(std::string_view& s) {
            printf("%.*s", (int)s.size(), s.data());
        }
        void operator()(Struct& s) {
            s.dump(depth);
        }
        void operator()(Array& a) {
            a.dump(depth);
        }
        void operator()(std::unique_ptr<Value>& v) {
            v->dump(depth);
        }
    };
    std::visit(Visitor{depth + 1}, exts);
    if (args) {
        printf("(");
        args->dump(depth + 1);
        printf(")");
    }
    if (concat.size() > 1) {
        for (auto& c : concat) {
            printf("\n");
            indent(depth);
            printf("+ ");
            std::visit(Visitor{depth + 1}, c);
        }
    } else {
        for (auto& c : concat) {
            printf(" + ");
            std::visit(Visitor{depth + 1}, c);
        }
    }
}

void Array::dump(int depth) {
    if (elements.size() > 2) {
        printf("[\n");
        for (auto& e : elements) {
            indent(depth);
            e.dump(depth + 1);
            printf("\n");
        }
        indent(depth - 1);
        printf("]");
    } else {
        printf("[");
        for (auto& e : elements) {
            e.dump(depth + 1);
        }
        printf("]");
    }
}
