#pragma once

#include <concepts>
#include <cstdlib>

#define ARENA_SZ 4096

template <class A>
concept Allocator = requires(A& a) {
    { a.alloc(sizeof(int)) } -> std::same_as<void*>;
};

class Arena {
  private:
    struct Segment {
        char* bump;
        char* end;
        Segment* next;
        char buf[ARENA_SZ - 24];
    };

    Segment* m_start;
    Segment* m_current;

    void grow();

  public:
    Arena() : m_start(nullptr), m_current(nullptr) {}
    Arena(Arena&) = delete;
    Arena& operator=(Arena&) = delete;
    Arena(Arena&& old) {
        m_start = old.m_start;
        m_current = old.m_current;
        old.m_start = nullptr;
        old.m_current = nullptr;
    }
    Arena& operator=(Arena&& old) {
        m_start = old.m_start;
        m_current = old.m_current;
        old.m_start = nullptr;
        old.m_current = nullptr;
        return *this;
    }
    ~Arena() {
        if (!m_start) return;
        auto current = m_start;
        while (current) {
            auto next = current->next;
            delete current;
            current = next;
        }
    }

    class Save {
        friend Arena;

        char* bump;
        Segment* seg;

        Save(Segment* segment) {
            if (!segment) {
                bump = nullptr;
                seg = nullptr;
            } else {
                bump = segment->bump;
                seg = segment;
            }
        }
    };

    void* alloc(size_t size);
    template <class T>
    T* place(T val) {
        return new (alloc(sizeof(val))) T(std::move(val));
    }
    Save save();
    void revert(Save);
    void revert();
};
