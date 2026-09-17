#pragma once

#include "cmd.hxx"

#include <ctime>
#include <fstream>
#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cy {

static constexpr std::string_view build_dir = "build";
struct Package;
struct Configuration;

class _St {
    static std::unordered_map<std::string, timespec> mtimes;
    static std::string localize_path(std::string path) {
        for (size_t i = 0; i < path.size(); i++) {
            if (path[i] == '/') path[i] = '_';
        }
        return path;
    }
    static std::string read_file(std::string& path) {
        std::ifstream file(path, std::ios::in | std::ios::ate);
        auto size = file.tellg();
        file.seekg(0);
        std::string content(size, '\0');
        file.read(content.data(), size);
        return content;
    }

    static void build_src(
        Cmd& cmd,
        Package& p,
        Configuration& conf,
        std::string source,
        bool& did_update
    ) {
        auto src = std::string{source};
        auto local_name = localize_path(std::move(source));
    }

    friend Package;
};

static inline int operator<=>(timespec& a, timespec& b) {
    auto v = a.tv_sec - b.tv_sec;
    v += (!v) * (a.tv_nsec - b.tv_nsec);
    return v;
}
static inline bool operator<(timespec& a, timespec& b) {
    return (a <=> b) < 0;
}
static inline bool operator>(timespec& a, timespec& b) {
    return (a <=> b) > 0;
}

static inline void wait_for_all() {
    while (true) {
        if (wait(NULL) < 0) break;
    }
}

struct Configuration {
    std::initializer_list<std::string_view> cflags;
    std::string_view name;
    bool force;
};

struct Package {
    std::initializer_list<std::string_view> cflags;
    std::initializer_list<std::string_view> sources;
    std::initializer_list<Package*> deps;
    std::string_view name;
    enum {
        Exe,
        Lib
    } kind;

    std::string out_path(Configuration& conf) {
        return std::string{}
            .append(build_dir)
            .append("/")
            .append(conf.name)
            .append("/")
            .append(name);
    }

    bool build(Configuration& conf) {
        bool did_update = false;
        Cmd cmd{};
        for (auto d : deps) {
            did_update |= d->build(conf);
        }
    }
};

} // namespace cy
