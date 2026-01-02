#pragma once
#include <cstddef>
#include <cstdint>

/*
 *Intrusive List -
 * Intrusive doubly linked list with following properties
 * - The list does not own elements
 * - The list never allocates
 * - The list never destroys objects
 * - object lifetime is controlled externally
 * - structural operations are pure pointer arithmetic
 * - all operation are deterministic O(1)
 *
 * This kind of design is popular in:
 * HFT Trading Systems
 * Operating system kernels
 * Game engines
 * Lock free / wait free pipelines
 */

/*
 *Intrusive Hook
 *The hook is directly embedded into user objects
 *The hook has no ownership semantics
 * It is purely structural
 * An object can only be in ONE list per hook
 * In real systems multiple hooks can exist per object
 * eg: Price-level list, time-priority list, free list, etc
 */

struct intrusive_hook
{
 intrusive_hook* prev = nullptr;
 intrusive_hook* next = nullptr;

 // lets add a guard
 [[nodiscard]] bool is_linked() const noexcept
 {
  return (prev != nullptr) && (next != nullptr);
 }

};

/*
 *Intrusive List
 * The list only stores pointers to the hooks
 * It does not know about the object type
 * Design decisions:
 * - Sentinel based circular list (no null checks)
 * - no templates for clarity and debuggability
 * - No exceptions, No allocators, No STL dependencies
 */

class intrusive_list
{
private:
 intrusive_hook sentinel_;

private:
 // LOW LEVEL HELPERS
 static void link_between(
  intrusive_hook* x,
  intrusive_hook* a,
  intrusive_hook* b
  ) noexcept
 {
  x->prev = a;
  x->next = b;
  a->next = x;
  b->prev = x;
 }

 static void unlink(
  intrusive_hook* x) noexcept
 {

  if (!x->is_linked()) return;

  x->prev->next = x->next;
  x->next->prev = x->prev;
  x->prev = nullptr;
  x->next = nullptr;
 }

public:
 // CONSTRUCTION
 intrusive_list() noexcept
 {
  sentinel_.prev = &sentinel_;
  sentinel_.next = &sentinel_;
 }

 intrusive_list(const intrusive_list&) = delete;

 intrusive_list& operator=(const intrusive_list&) = delete;

 // BASIC PROPERTIES
 [[nodiscard]] bool empty() const noexcept
 {
  return sentinel_.next == &sentinel_;
 }

 intrusive_hook* front() noexcept
 {
  return sentinel_.next;
 }
 intrusive_hook* back() noexcept
 {
  return sentinel_.prev;
 }
 intrusive_hook* end() noexcept
 {
  return &sentinel_;
 }

 // add clear
 void clear() noexcept
 {
  while (!empty())
  {
   remove(front());
  }
 }

 // INSERTION
 void push_front(intrusive_hook* h) noexcept
 {
  link_between(h,&sentinel_,sentinel_.next);
 }
 void push_back(intrusive_hook* h) noexcept
 {
  link_between(h,sentinel_.prev, &sentinel_);
 }

 // REMOVAL
 void remove(intrusive_hook* h) noexcept
 {
  unlink(h);
 }

 // SPLICE
 /* moves node h before pos
  * no allocation, no destruction, no ownership transfer
  * constant number of pointer writes, deterministic latency
  *
  * used in LRU promotion, priority reordering, time ordering, queue discipline, etc
  */

 void splice(intrusive_hook* pos, intrusive_hook* h) noexcept
 {
  if (h==pos) return;
  unlink(h);
  link_between(h, pos->prev, pos);
 }

 // range splice - [first,last) before pos

 void splice(intrusive_hook* pos, intrusive_hook* first, intrusive_hook* last) noexcept
 {
  if (first == last) return;
  intrusive_hook* tail = last->prev;
  // detach [first,tail]
  first->prev->next = last;
  last->prev = first->prev;

  // attach before pos
  intrusive_hook* before = pos->prev;
  before->next = first;
  first->prev = before;

  tail->next = pos;
  pos->prev = tail;
 }

};

