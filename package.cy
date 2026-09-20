Package {
    name: cy
    -- sources: [src/main.cxx]
    sources: glob(src/*.cxx) + [config.cxx]
    c:flags: [-Wall -Wextra -std=c++23]
    -- c: {
    --     compiler: cc
    --     flags: [-Wall -Wextra -std=c++23]
    -- }
}

Configuration {
    name: debug
}

Configuration {
    name: release
    c:flags: [-O3]
    force: true
}

Configuration {
    name: native
    c:flags: release.c.flags + [-march=native]
    force: release.force
}

-- File     := Expr*
-- Expr     := Value AggValue? Concat*
-- Concat   := '+' Value
-- Value    := ident / AggValue
-- AggValue := Parens / Array / Struct
-- Struct   := '{' Field* '}'
-- Field    := ident ':' (Expr / Field)
-- Array    := '[' Expr* ']'
-- Parens   := '(' Expr ')'
