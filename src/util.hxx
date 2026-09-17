#pragma once

#include <fstream>
#include <string>

inline std::string read_file(std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::ate);
    auto size = file.tellg();
    file.seekg(0);
    std::string content(size, '\0');
    file.read(content.data(), size);
    return content;
}
