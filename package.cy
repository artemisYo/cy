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

-- File   := Struct+
-- Struct := ident? '{' Field+ '}'
-- Field  := ident ':' (Value / Field)
-- Value  := PValue (Parens / Array)? Concat*
-- PValue := ident / Struct / Array / Parens
-- Array  := '[' Value* ']'
-- Parens := '(' Value ')'
-- Concat := '+' PValue
