#pragma once

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <expected>
#include <string_view>
#include <unistd.h>

namespace cy {

class Cmd {
  public:
    struct Options {
        int* stdout_fd = nullptr;
        int* stderr_fd = nullptr;
        pid_t* pid_out = nullptr;
        bool wait = false;
    };

    std::expected<void, int> execp(Options opt) {
        if (opt.stdout_fd) {
            dup2(*opt.stdout_fd, STDOUT_FILENO);
        }
        if (opt.stderr_fd) {
            dup2(*opt.stderr_fd, STDERR_FILENO);
        }
        execvp(m_buf[0], m_buf);
        return std::unexpected{errno};
    }
    std::expected<void, int> execfp(Options opt) {
        pid_t pid = fork();
        switch (pid) {
            case -1: {
                return std::unexpected{errno};
            }
            case 0: {
                auto err = this->execp(opt);
                exit(err.error());
            }
            default: {
                if (opt.pid_out) *opt.pid_out = pid;
                if (opt.wait) waitpid(pid, NULL, 0);
                return std::expected<void, int>{};
            }
        }
    }

    size_t size() const {
        return this->m_len;
    }

    void push(std::string_view str) {
        this->reserve(m_len + 2);
        push_assume_capacity(str);
    }
    template <class... Args>
    void pushv(Args&&... strs) {
        this->reserve(m_len + sizeof...(strs) + 1);
        (this->push_assume_capacity(strs), ...);
    }

    void truncate(size_t length) {
        if (m_len <= length) return;
        for (size_t i = length; i < m_len; i++) {
            auto s = m_buf[i];
            delete s;
        }
        m_len = length;
        m_buf[m_len] = nullptr;
    }

    char** begin() {
        return &m_buf[0];
    }
    char** end() {
        return &m_buf[m_len];
    }

    Cmd() : m_buf(nullptr), m_cap(0), m_len(0) {}
    Cmd(Cmd&) = delete;
    Cmd& operator=(Cmd&) = delete;
    Cmd(Cmd&& old) {
        m_buf = old.m_buf;
        old.m_buf = nullptr;
        m_len = old.m_len;
        old.m_len = 0;
        m_cap = old.m_cap;
        old.m_cap = 0;
    }
    Cmd& operator=(Cmd&& old) {
        m_buf = old.m_buf;
        old.m_buf = nullptr;
        m_len = old.m_len;
        old.m_len = 0;
        m_cap = old.m_cap;
        old.m_cap = 0;
        return *this;
    }
    ~Cmd() {
        if (!m_buf) return;
        for (size_t i = 0; i < m_len; ++i) {
            auto s = m_buf[i];
            delete s;
        }
        delete m_buf;
    }

  private:
    char** m_buf;
    size_t m_cap;
    size_t m_len;

    void push_assume_capacity(std::string_view str) {
        char* owned = new char[str.size() + 1];
        str.copy(owned, -1);
        m_buf[m_len++] = owned;
        m_buf[m_len] = nullptr;
    }

    void reserve(size_t length) {
        if (!m_buf) {
            m_cap = length;
            m_buf = new char*[m_cap];
            return;
        }

        if (length >= m_cap) {
            m_cap = length * 2;
            auto tmp = new char*[m_cap];
            memcpy(tmp, m_buf, m_len * sizeof(*m_buf));
            delete m_buf;
            m_buf = tmp;
            return;
        }
    }
};

} // namespace cy
