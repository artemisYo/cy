#pragma once

#include <cassert>

template <class T>
class List {
  public:
    struct Node {
        Node* next;
        T value;

        Node(T val) : next(nullptr), value(val) {}
    };

    struct Iter {
        Node* current;

        Iter& operator++() {
            this->current = this->current->next;
            return *this;
        }
        bool operator==(Iter& other) {
            return this->current == other.current;
        }
        T& operator*() {
            return this->current->value;
        }

        size_t count() {
            Iter head = *this;
            size_t count = 0;
            while (head.current) ++head, ++count;
            return count;
        }
    };

    List() : m_head(nullptr), m_tail(nullptr) {}

    Node* append(Node* n) {
        assert(!m_head == !m_tail);
        if (m_tail) {
            m_tail->next = n;
            m_tail = n;
        } else {
            m_head = n;
            m_tail = n;
        }
        return n;
    }
    Node* prepend(Node* n) {
        assert(!m_head == !m_tail);
        if (!m_tail) m_tail = n;
        n->next = m_head;
        m_head = n;
        return n;
    }

    bool empty() const {
        return m_head == nullptr;
    }

    T& front() {
        return m_head->value;
    }
    T& back() {
        return m_tail->value;
    }

    Node* pop_front() {
        Node* out = m_head;
        m_head = out->next;
        if (!m_head) m_tail = nullptr;
        return out;
    }

    Iter begin() {
        return Iter(m_head);
    }
    Iter end() {
        return Iter(nullptr);
    }

  private:
    Node* m_head;
    Node* m_tail;
};
