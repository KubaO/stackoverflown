// https://github.com/KubaO/stackoverflown/tree/master/questions/circ-iterator-9993713
#include <boost/iterator/iterator_facade.hpp>
#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>
#include <limits>
#include <iostream>
#include <cassert>
#include <cstdint>
#include <algorithm>

template <typename T, bool merge_tag = false, typename tag_type = uint8_t> class tagged_ptr;

template <typename T, typename tag_type> class tagged_ptr<T, true, tag_type> {
    uintptr_t ptr;
    typedef std::numeric_limits<uintptr_t> lim;
    inline static uintptr_t ptr_of(T* p) {
        assert(tag_of(p) == 0);
        return uintptr_t(p);
    }
    inline static uintptr_t tag_mask() { return 3; }
    inline uintptr_t ptr_only() const { return ptr & (lim::max() - tag_mask()); }
    inline static tag_type tag_of(T* p) { return ((tag_type)(uintptr_t)p) & tag_mask(); }
    inline tag_type tag_only() const { return ptr & tag_mask(); }
public:
    tagged_ptr(T* p, tag_type t) : ptr(ptr_of(p) | t) { assert(t <= tag_mask()); }
    tagged_ptr(const tagged_ptr & other) : ptr(other.ptr) {}
    operator T*() const { return reinterpret_cast<T*>(ptr_only()); }
    T* operator->() const { return reinterpret_cast<T*>(ptr_only()); }
    tagged_ptr & operator=(T* p) { ptr = tag_only() | ptr_of(p); return *this; }
    tag_type tag() const { return tag_only(); }
    void set_tag(tag_type tag) { assert(tag <= tag_mask()); ptr = tag | ptr_only(); }
};

template <typename T, typename tag_type> class tagged_ptr<T, false, tag_type> {
    T* ptr;
    tag_type m_tag;
public:
    tagged_ptr(T* p, tag_type t) : ptr(p), m_tag(t) {}
    tagged_ptr(const tagged_ptr & other) : ptr(other.ptr), m_tag(other.m_tag) {}
    operator T*() const { return ptr; }
    T* operator->() const { return ptr; }
    tagged_ptr & operator=(T* p) { ptr = p; return *this; }
    tag_type tag() const { return m_tag; }
    void set_tag(tag_type tag) { m_tag = tag; }
};

struct circ : private boost::noncopyable {
    unsigned int i;
    circ* next;
    circ* prev;
    explicit circ(int i) : i(i), next(nullptr), prev(nullptr) {}
    circ(int i, circ& prev) : i(i), next(nullptr), prev(&prev) {
        prev.next = this;
    }
    circ(int i, circ& prev, circ& next) : i(i), next(&next), prev(&prev) {
        prev.next = this;
        next.prev = this;
    }
};

class circ_iterator;
circ_iterator end(circ& c);

class circ_iterator
        : public boost::iterator_facade<
        circ_iterator, circ, boost::bidirectional_traversal_tag
        >
{
    tagged_ptr<circ> c;
    enum { Default, Inc, End };
    friend class boost::iterator_core_access;
    friend circ_iterator end(circ&);
    struct end {};
    circ_iterator(circ& c_, end) : c(&c_, End) {}

    circ& dereference() const { return *c; }
    void increment() {
        c = c->next;
        if (c.tag() != End) c.set_tag(Inc);
    }
    void decrement() {
        c = c->prev;
        if (c.tag() != End) c.set_tag(Default);
    }
    bool equal(const circ_iterator & other) const {
        return this->c == other.c &&
                (other.c.tag() != End || this->c.tag() != Default);
    }
public:
    circ_iterator() : c(nullptr, Default) {}
    circ_iterator(circ& c_) : c(&c_, Default) {}
    circ_iterator(const circ_iterator& other) : c(other.c) {}
};

circ_iterator begin(circ& c) { return circ_iterator(c); }
circ_iterator end(circ& c) { return circ_iterator(c, circ_iterator::end()); }

int main()
{
    circ a(0), b(1, a), c(2, b), d(3, c, a);
    assert(end(a) == end(a));
    assert(++--end(a) == end(a));
    for (auto it = begin(a); it != end(a); ++it) {
        std::cout << it->i << std::endl;
    }
    std::cout << "*" << std::endl;
    for (auto it = ++begin(a); it != --end(a); ++it) {
        std::cout << it->i << std::endl;
    }
    std::cout << "*" << std::endl;
    for (auto & c : a)
        std::cout << c.i << std::endl;
}
