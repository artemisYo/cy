#include "parser.hxx"
#include "tokenizer.hxx"
#include "util.hxx"

#include <cstdio>
#include <duration.hxx>

void timed(const char* label, auto f) {
    auto start = cy::Time::now();
    f();
    auto end = cy::Time::now();
    printf("> %s: ", label);
    (end - start).print();
    printf("\n");
}

int main() {
    Arena ar;
    auto path = "package.cy";
    std::string input{};
    timed("read", [&]() {
        size_t len = 0;
        char* content = read_file(path, len);
        input = std::string{content, len};
        delete content;
    });

    timed("lex", [&]() {
        auto lex = Tokenizer(input);
        Token t{};
        while (lex.next(t)) {
            (void)t;
        }
    });

    File file{};
    bool success = true;
    Parser p{input, ar};
    timed("parse", [&]() {
        success = p.file(file);
    });
    if (!success) {
        printf("%s", p.diagnostic().c_str());
        return 1;
    }
    timed("dump", [&]() {
        file.dump();
    });

    return 0;
}
