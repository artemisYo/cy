#pragma once

#include <cassert>
#include <cstdio>

inline char* read_file(const char* path, size_t& len) {
    FILE* file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* str = new char[len + 1];
    assert(fread(str, 1, len, file) == (unsigned long)len);
    str[len] = 0;
    return str;
}
