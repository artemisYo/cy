#include "parser.hxx"

#include "ast.hxx"

#include <cstdio>

template <class T>
inline List<T>::Node list_node(T t) {
    return typename List<T>::Node(t);
}

struct PParser : Parser {
    PParser(Parser&& p) : Parser(p) {}

    bool pfile(File& out) {
        while (!lex.done()) {
            auto res = out.items.append(ar.place(list_node(Expr{})));
            if (!pexpr(res->value)) return false;
        }
        return true;
    }

    bool pexpr(Expr& out) {
        out.value = ar.place(Value{});
        if (!pvalue(*out.value)) return false;

        if (lex.has(Token::open_paren)) {
            out.arg = ar.place(Value{});
            out.arg->kind = Value::kexpr;
            out.arg->expr = Expr{};
            if (!pparens(out.arg->expr)) return false;
        } else if (lex.has(Token::open_bracket)) {
            out.arg = ar.place(Value{});
            out.arg->kind = Value::karray;
            out.arg->array = Array{};
            if (!parray(out.arg->array)) return false;
        } else if (lex.has(Token::open_brace)) {
            out.arg = ar.place(Value{});
            out.arg->kind = Value::kstruct;
            out.arg->ustruct = Struct{};
            if (!pstruct(out.arg->ustruct)) return false;
        }

        Token t;
        while (lex.peek(t) && lex.str_of(t) == "+") {
            lex.next(t);
            auto res = out.concat.append(ar.place(list_node(Value{})));
            if (!pvalue(res->value)) return false;
        }

        return true;
    }

    bool pvalue(Value& out) {
        if (lex.done()) return false;

        Token t;
        lex.peek(t);
        switch (t.kind) {
            default: return false;
            case Token::identifier: {
                lex.next(t);
                out.kind = Value::kident;
                out.ident = lex.str_of(t);
                return true;
            }
            case Token::open_brace: {
                out.kind = Value::kstruct;
                out.ustruct = Struct{};
                return pstruct(out.ustruct);
            }
            case Token::open_bracket: {
                out.kind = Value::karray;
                out.array = Array{};
                return parray(out.array);
            }
            case Token::open_paren: {
                out.kind = Value::kexpr;
                out.expr = Expr{};
                return pparens(out.expr);
            }
        }
    }

    bool pstruct(Struct& out) {
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
        out.value = Expr{};
        if (lookahead.eat_if(Token::colon)) {
            out.value.value = ar.place(Value{});
            out.value.value->kind = Value::kstruct;
            out.value.value->ustruct = Struct{};
            auto f = out.value.value->ustruct.fields.append(
                ar.place(list_node(Field{}))
            );
            if (!pfield(f->value)) return false;
        } else {
            if (!pexpr(out.value)) return false;
        }

        return true;
    }

    bool parray(Array& out) {
        if (!lex.eat_if(Token::open_bracket)) return false;
        while (!lex.eat_if(Token::close_bracket)) {
            auto res = out.elements.append(ar.place(list_node(Expr{})));
            if (!pexpr(res->value)) return false;
        }

        return true;
    }

    bool pparens(Expr& out) {
        if (!lex.eat_if(Token::open_paren)) return false;
        if (!pexpr(out)) return false;
        if (!lex.eat_if(Token::close_paren)) return false;

        return true;
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
    value.dump(depth);
}

void Expr::dump(int depth) {
    value->dump(depth);
    if (arg) {
        if (arg->kind == Value::kexpr) {
            printf("(");
            arg->dump(depth + 1);
            printf(")");
        } else {
            arg->dump(depth);
        }
    }
    if (!concat.empty()) {
        for (auto& c : concat) {
            printf("\n");
            indent(depth);
            printf("+ ");
            c.dump(depth);
        }
    } else {
        for (auto& c : concat) {
            printf(" + ");
            c.dump(depth);
        }
    }
}

void Value::dump(int depth) {
    switch (kind) {
        case Value::kident:
            printf("%.*s", (int)ident.size(), ident.data());
            break;
        case Value::kstruct:
            ustruct.dump(depth);
            break;
        case Value::karray:
            array.dump(depth);
            break;
        case Value::kexpr:
            expr.dump(depth);
            break;
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
