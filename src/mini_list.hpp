#pragma once
#include <cstddef>
#include <iterator>
#include <utility>

template <typename T>
class mini_list
{
private:
    struct node
    {
        T value;
        node* prev;
        node* next;

        node(const T& v) : value(v), prev(nullptr), next(nullptr) {}
        node(T&& v) : value(std::move(v)), prev(nullptr), next(nullptr) {}
    };

    node* head; // sentinel
    node* tail; // sentinel
    size_t sz;

public:
    class iterator
    {
        node* ptr;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(node* p = nullptr) : ptr(p) {}

        T& operator*{} const {return ptr->value;}
        iterator& operator++() {ptr = ptr->next; return *this;}
        iterator& operator--() {ptr = ptr->prev; return *this;}

        bool operator==(const iterator& o) const {return ptr == o.ptr;}
        bool operator!=(const iterator& o) const {return ptr != o.ptr;}

        friend class mini_list<T>;
    };

    mini_list() : sz(0)
    {
        head = new node(T{});
        tail = new node(T{});
        head->next = tail;
        tail->prev = head;
    }

    ~mini_list()
    {
        clear();
        delete head;
        delete tail;
    }

    void clear()
    {
        node* cur = head->next;
        while (cur != tail)
        {
            node* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
        head->next = tail;
        tail->prev = head;
        sz = 0;
    }

    size_t size() const {return sz;}

    iterator begin() {return iterator(head->next);}
    iterator end() {return iterator(tail);}

    iterator insert(iterator pos, const T& val)
    {
        node* n = new node(val);
        node* p = pos.ptr;

        n->prev = p->prev;
        n->next = p;
        p->prev->next = n;
        p->prev = n;

        ++sz;
        return iterator(n);
    }

    iterator erase(iterator pos)
    {
        node* n = pos.ptr;
        iterator ret(n->next);

        n->prev->next = n->next;
        n->next->prev = n->prev;
        delete n;
        sz--;

        return ret;
    }

    void push_back(const T& val) {insert(end(),val());}
};