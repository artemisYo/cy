#include "arena.hxx"

void Arena::grow() {
    if (!m_current) {
        m_start = new Segment;
        m_current = m_start;
    } else {
        m_current->next = new Segment;
        m_current = m_current->next;
    }
    m_current->next = nullptr;
    m_current->bump = m_current->buf;
    m_current->end = &m_current->bump[sizeof(m_current->buf)];
}

void* Arena::alloc(usize size) {
    if (!this->m_current) this->grow();
    auto old_bump = (uintptr_t)m_current->bump;
    old_bump = (old_bump + 15) & (~15);
    auto new_bump = old_bump + size;
    if ((char*)new_bump >= m_current->end) {
        this->grow();
        return this->alloc(size);
    }
    m_current->bump = (char*)new_bump;
    return (void*)old_bump;
}

Arena::Save Arena::save() {
    return Arena::Save(m_current);
}

void Arena::revert(Save s) {
    if (!s.seg) {
        this->revert();
        return;
    }
    m_current = s.seg;
    m_current->bump = s.bump;
}

void Arena::revert() {
    m_current = m_start;
    m_current->bump = m_start->buf;
}
