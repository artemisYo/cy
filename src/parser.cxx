#include "parser.hxx"

#include <cstdio>

template <class T>
inline List<T>::Node list_node(T t) {
    return typename List<T>::Node(t);
}

struct PParser : Parser {
    PParser(Parser&& p) : Parser(p) {}

    bool pfile(File& out) {
        auto res = out.items.append(ar.place(list_node(Struct{})));
        if (!pstruct(res->value)) return false;
        while (!lex.done()) {
            auto res = out.items.append(ar.place(list_node(Struct{})));
            if (!pstruct(res->value)) return false;
        }
        return true;
    }

    bool pstruct(Struct& out) {
        Token t;
        if (lex.next_if(Token::identifier, t)) {
            out.name = lex.str_of(t);
        } else {
            out.name = "";
        }

        if (!lex.eat_if(Token::open_brace)) return false;

        while (!lex.eat_if(Token::close_brace)) {
            auto res = out.fields.append(ar.place(list_node(Field{})));
            if (!pfield(res->value)) return false;
        }

        return true;
    }

    bool pfield(Field& out) {
        Token t;
        if (!lex.next_if(Token::identifier, t)) return false;
        out.key = lex.str_of(t);

        if (!lex.eat_if(Token::colon)) return false;

        auto lookahead = lex;
        lookahead.eat();
        if (lookahead.eat_if(Token::colon)) {
            out.value = ar.place(Field{});
            if (!pfield(*std::get<0>(out.value))) return false;
        } else {
            out.value = Value{};
            if (!pvalue(std::get<1>(out.value))) return false;
        }

        return true;
    }

    bool parray(Array& out) {
        if (!lex.eat_if(Token::open_bracket)) return false;
        while (!lex.eat_if(Token::close_bracket)) {
            auto res = out.elements.append(ar.place(list_node(Value{})));
            if (!pvalue(res->value)) return false;
        }

        return true;
    }

    bool pparens(Value& out) {
        if (!lex.eat_if(Token::open_paren)) return false;
        if (!pvalue(out)) return false;
        if (!lex.eat_if(Token::close_paren)) return false;

        return true;
    }

    bool pvalue(Value& out) {
        if (!ppure_value(out.exts)) return false;

        if (lex.has(Token::open_paren)) {
            out.args = ar.place(Value{});
            if (!pparens(*out.args)) return false;
        } else if (lex.has(Token::open_bracket)) {
            out.args = ar.place(Value{});
            out.args->exts = Array{};
            if (!parray(std::get<2>(out.args->exts))) return false;
        }

        Token t;
        while (lex.peek(t) && lex.str_of(t) == "+") {
            lex.next(t);
            auto res = out.concat.append(ar.place(list_node(Value::Exts{})));
            if (!ppure_value(res->value)) return false;
        }

        return true;
    }

    bool ppure_value(Value::Exts& out) {
        if (lex.done()) return false;

        Token t;
        lex.peek(t);
        switch (t.kind) {
            default: return false;
            case Token::identifier: {
                lex.next(t);
                out = lex.str_of(t);
                return true;
            }
            case Token::open_brace: {
                out = Struct{};
                return pstruct(std::get<1>(out));
            }
            case Token::open_bracket: {
                out = Array{};
                return parray(std::get<2>(out));
            }
            case Token::open_paren: {
                out = ar.place(Value{});
                return pparens(*std::get<3>(out));
            }
        }
    }
};

bool Parser::file(File& out) {
    PParser p{std::move(*this)};
    if (!p.pfile(out)) return false;
    new (this) Parser{std::move(p)};
    return true;
}

std::string Parser::diagnostic() {
    std::string s = "Unexpected token '";
    Token t;
    lex.peek(t);
    s.append(t.str_repr())
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
    if (!name.empty()) {
        printf("%.*s ", (int)name.size(), name.data());
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

        void operator()(Field*& f) {
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
        void operator()(Value*& v) {
            v->dump(depth);
        }
    };
    std::visit(Visitor{depth + 1}, exts);
    if (args) {
        printf("(");
        args->dump(depth + 1);
        printf(")");
    }
    if (!concat.empty()) {
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
    if (elements.begin().count() > 2) {
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
