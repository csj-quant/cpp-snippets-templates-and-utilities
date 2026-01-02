#pragma once
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

/*
 *Low Latency List + Pool
 * In this file we implement a nonintrusive doubly linked list backed by a fixed object pool.
 * Key properties by design:
 * - no dynamic allocation on hot paths
 * - deterministic memory layout
 * - explicit object lifetime
 * - O(1) structural operations
 * - std::list-style splice semantics
 * - stable node addresses
 * - suitable for latency sensitive applications
 */

// Node layout
template <typename T>
class ll_list_pool
{
private:
    struct node
    {
        node* prev;
        node* next;
        T value;
    };

// Sentinel node
    // lets use a classic circular sentinel node to avoid edge case branching
    // sentinel.prev points to last element
    // sentinel.next points to first element

    // when empty: sentinel.prev == sentinel.next == &sentinel
    // this mirrors how real std::list implementation works

    node sentinel_;

// Object pool state
    // - slab_ : contiguous block of nodes
    // - free_ : singly-linked free list (using node::next)
    // - cap_ : total node capacity
    // - size_ : number of live elements in list

    // NOTE: slab_ is allocated once, no memory returned to
    // OS until destruction

    node* slab_;
    node* free_;
    std::size_t cap_;
    std::size_t size_;

private:
// Internal helpers

    static node* as_node(void* p) noexcept
    {
        return static_cast<node*>(p);
    }

    // link x between a and b: a <-> x <-> b
    static void link_between(node* x, node* a, node* b) noexcept
    {
        x->prev = a;
        x->next = b;
        a->next = x;
        b->prev = x;
    }

    // unlink x from the list: a-x-b => a-b
    static void unlink(node* x) noexcept
    {
        x->prev->next = x->next;
        x->next->prev = x->prev;
    }

    // Allocate a node from the free list
    // NOTE: No construction of T happens here
    // pure pointer manipulation
    node* alloc_node()
    {
        if (!free_)
        {
            // pool exhausted: deterministic failure
            // in real systems this would trigger:
            // presizing, backpressure, or fatal error
            throw std::bad_alloc();
        }
        node* n = free_;
        free_ = free_->next;
        return n;
    }

    // Return a node back to the free list
    // NOTE: caller must destroy T beforehand
    void free_node(node* n) noexcept
    {
        n->next = free_;
        free_ = n;
    }

public:
// Iterator - very thin wrapper around the node
    class iterator
    {
        friend class ll_list_pool;
        node* n_;
        explicit iterator(node* n) noexcept : n_(n) {}
        public:
        iterator() noexcept : n_(nullptr) {}
        T& operator*() const noexcept
        {
            return n_->value;
        }
        T* operator->() const noexcept
        {
            return &n_->value;
        }
        iterator& operator++() noexcept
        {
            n_ = n_->next;
            return *this;
        }
        iterator& operator--() noexcept
        {
            n_ = n_->prev;
            return *this;
        }

        bool operator==(const iterator& o) const noexcept
        {
            return n_ == o.n_;
        }
        bool operator!=(const iterator& o) const noexcept
        {
            return n_ != o.n_;
        }
    };

public:
// Construction/Destruction
    explicit ll_list_pool(std::size_t capacity)
        :slab_(nullptr)
        , free_(nullptr)
        , cap_(capacity)
        , size_(0)
    {
        // allocate contigous slab for nodes
        slab_ = static_cast<node*>(
            ::operator new(sizeof(node)*cap_, std::align_val_t(alignof(node))));
        // build free list
        for (std::size_t i = 0; i < cap_; ++i)
        {
            slab_[i].next = free_;
            free_ = &slab_[i];
        }

        // initialise the sentinel
        sentinel_.prev = &sentinel_;
        sentinel_.next = &sentinel_;
    }

    ll_list_pool(const ll_list_pool&) = delete;
    ll_list_pool& operator=(const ll_list_pool&) = delete;

    ~ll_list_pool()
    {
        clear();
        ::operator delete(slab_, std::align_val_t(alignof(node)));
    }

// Basic properties

    bool empty() const noexcept
    {
        return size_ == 0;
    }
    std::size_t size() const noexcept
    {
        return size_;
    }
    iterator begin() noexcept
    {
        return iterator(sentinel_.next);
    }
    iterator end() noexcept
    {
        return iterator(&sentinel_);
    }

// Clear list

    // destroys all values and returns nodes to pool
    // deterministic O(n)

    void clear() noexcept
    {
        node* cur = sentinel_.next;
        node* end = &sentinel_;

        while (cur != end)
        {
            node* next = cur->next;
            cur->value.~T();
            free_node(cur);
            cur = next;
        }

        sentinel_.prev = &sentinel_;
        sentinel_.next = &sentinel_;
        size_ = 0;
    }

// Emplacement
    template <typename... Args>
    iterator emplace_front(Args&&... args)
    {
        node* n = alloc_node();
        ::new (&n->value) T(std::forward<Args>(args)...);
        link_between(n, &sentinel_, sentinel_.next);
        ++size_;
        return iterator(n);
    }

    template <typename... Args>
    iterator emplace_back(Args&&... args)
    {
        node* n = alloc_node();
        ::new (&n->value) T(std::forward<Args>(args)...);
        link_between(n, sentinel_.prev, &sentinel_);
        ++size_;
        return iterator(n);
    }

// Erase
    iterator erase(iterator it) noexcept
    {
        node* n = it.n_;
        iterator next(n->next);
        unlink(n);
        n->value.~T();
        free_node(n);
        --size_;
        return next;
    }

// MOST IMPORTANT: Splice
/* moves node 'what' before 'pos'.
 * properties:
 * no allocation
 * no deallocation
 * no value movement
 * pure pointer rewiring
 * O(1), deterministic
 */
    void splice(iterator pos, iterator what) noexcept
    {
        node* x = what.n_;
        if (x == pos.n_) return;

        unlink(x);
        link_between(x, pos.n_->prev, pos.n_);
    }

    // splice range [first,last) before pos
    void splice(iterator pos, iterator first, iterator last) noexcept
    {
        node* a = first.n_;
        node* b = last.n_;

        if (a == b) return;
        node* tail = b->prev;

        // detach [a,tail]
        a->prev->next = b;
        b->prev = a->prev;

        // attach before pos
        node* before = pos.n_->prev;
        before->next = a;
        a->prev = before;

        tail->next = pos.n_;
        pos.n_->prev = tail;
    }
};