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
    std::string path = "package.cy";
    std::string input{};
    timed("read", [&]() {
        input = read_file(path);
    });

    timed("lex", [&]() {
        auto lex = Tokenizer(input);
        while (auto t = lex.next()) {
            (void)t;
        }
    });

    return 0;
}
