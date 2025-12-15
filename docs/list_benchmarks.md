### Benchmark Results: `std::vector` vs `std::list`

This document records empirical performance measurements comparing `std::vector` and `std::list` for insertion, erasure, and structural movement operations. All benchmarks were compiled with C++23, optimizations enabled (`-O3`), and executed on a Linux system using `CLOCK_MONOTONIC_RAW` for timing.

---

### 1. Middle Insertion Benchmark

Operation: Insert a single element at the middle of a container containing 200,000 elements.

Results:

* vector insert: **0.846392 ms**
* list insert: **0.001913 ms**

Analysis:

Inserting into the middle of a `std::vector` requires shifting all subsequent elements to make room for the new value. This involves a large contiguous memory move, which is expensive despite excellent cache locality. The operation is O(n) with a large constant factor due to memory bandwidth usage.

In contrast, `std::list` performs insertion by allocating a single node and rewiring a constant number of pointers. No existing elements are moved. As a result, the operation completes in O(1) time and is over **400× faster** in this specific scenario.

This benchmark highlights the *only* class of problems where linked lists outperform vectors: frequent structural modifications in the middle of a sequence.

---

### 2. Middle Erase Benchmark

Operation: Erase a single element from the middle of a container containing 200,000 elements.

Results:

* vector erase: **0.128826 ms**
* list erase: **0.013289 ms**

Analysis:

Erasing an element from a `std::vector` again requires shifting all elements that follow the erased position to close the gap. Although faster than insertion (no new allocation), this still involves a linear-time memory move.

`std::list` erases a node by unlinking it from its neighbors and deallocating it. The cost is constant and independent of container size.

While the gap is smaller than in insertion, `std::list` remains roughly **10× faster** for single-element erasure in the middle.

This demonstrates that lists are advantageous for isolated structural deletions but not necessarily for bulk operations.

---

### 3. Structural Transfer: `list::splice` vs `vector` erase + insert

Operation: Move one element from one container to another.

Results:

* list splice: **5,819 ns**
* vector erase + insert: **2,128,066 ns**

Analysis:

`std::list::splice` is a unique operation in the STL. It transfers nodes between lists by relinking pointers only. No allocation, no deallocation, and no element movement occurs. The operation is strictly O(1).

Achieving the same logical operation with vectors requires:

* Erasing the element from the source vector (O(n))
* Inserting the element into the destination vector (O(n))

This results in a cost that grows linearly with container size and is dominated by memory movement. In this benchmark, vector-based movement is over **350× slower** than `list::splice`.

This experiment explains why `std::list` still exists in the STL: it enables zero-copy, constant-time structural transfers that are impossible with contiguous containers.

---

### Key Observations

* `std::vector` dominates when iteration, traversal, and cache locality matter.
* `std::list` excels only in scenarios involving frequent middle insertions, erasures, or structural transfers.
* `list::splice` is the most compelling feature of linked lists and has no vector equivalent.
* Despite favorable asymptotic complexity, linked lists generally underperform in real systems due to poor cache locality and allocator overhead.
* Low-latency and HFT systems avoid lists unless structural operations outweigh iteration costs.

---

### Practical Takeaway

Use `std::vector` as the default container.
Use `std::list` **only** when constant-time structural operations such as `splice`, frequent middle insertions, or stable iterators are core requirements of the algorithm.

---
