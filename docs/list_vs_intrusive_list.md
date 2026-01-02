# List vs Intrusive List  
## Low-Latency Splice & Traversal Benchmark (C++23)

This document presents empirical results comparing a **pool-backed non-intrusive doubly linked list** against a **fully intrusive doubly linked list**, implemented from scratch with low-latency constraints in mind.

The goal is not to compare Big-O complexity — both structures are O(1) for structural operations — but to understand **instruction-level cost, cache behavior, and latency characteristics** that matter in trading systems and other real-time environments.

---

## 1. Experimental Setup

### Hardware & Runtime
- Architecture: x86-64
- Compiler: `g++` (C++23)
- Build: `-O3`
- Timing source: `std::chrono::steady_clock`
- Execution: single-threaded
- Memory: preallocated, no runtime allocation on hot paths

### Test Parameters

```cpp
N_SMALL = 10
N_LARGE = 1,000,000
OPS     = 5,000,000
````

### Data Structures Tested

#### Pool-Backed Non-Intrusive List

* Fixed-capacity object pool
* Stable node addresses
* `std::list`-style circular sentinel
* No allocation/deallocation on splice
* Node layout:

  ```cpp
  struct node {
      node* prev;
      node* next;
      T     value;
  };
  ```

#### Intrusive List

* No ownership of elements
* No allocation at any time
* Hooks embedded directly in user objects
* Pure pointer rewiring
* Node layout:

  ```cpp
  struct intrusive_hook {
      intrusive_hook* prev;
      intrusive_hook* next;
  };
  ```

---

## 2. Correctness Sanity Check

A small demonstration with 10 elements verifies correct ordering and splice behavior.

### Operation

```text
splice(last → front)
```

### Result (both implementations)

```text
Initial: 0 1 2 3 4 5 6 7 8 9
After:   9 0 1 2 3 4 5 6 7 8
```

This confirms:

* Sentinel logic correctness
* Valid pointer rewiring
* No structural corruption

---

## 3. Benchmark 1 — Full Traversal (Pointer Chasing)

### What is measured

* Sequential traversal cost
* Pointer dereference overhead
* Cache behavior during iteration

### Code Pattern

```cpp
for (auto it = begin; it != end; ++it)
    sum += value;
```

### Results

```text
Pool list traversal (ns):      55
Intrusive list traversal (ns): 29
```

### Interpretation

Although both are linked lists, intrusive traversal is faster due to:

* Fewer pointer indirections
* Tighter memory layout
* Payload and linkage co-located
* Reduced cache-line pressure

This reflects **lower per-node instruction cost**, not algorithmic differences.

---

## 4. Benchmark 2 — Repeated Splice (Hot Path)

### What is measured

* Promotion cost under random access
* Constant-time structural modification
* Realistic LRU-style behavior

### Workload

* 5,000,000 splice operations
* Random element promotion
* No allocation or destruction in loop

### Results

```text
Pool list splice (ns):      315,644,996
Intrusive list splice (ns): 219,499,593
```

### Normalized Cost

| Structure      | Total Time | Avg per splice |
| -------------- | ---------- | -------------- |
| Pool list      | 315 ms     | ~63 ns         |
| Intrusive list | 219 ms     | ~44 ns         |

### Key Observation

> Intrusive splice is ~30% faster on the hot path.

Both are O(1), but **constant factors dominate** in low-latency systems.

---

## 5. Why Intrusive Lists Win on the Hot Path

### Pool-Backed List

* Larger node footprint
* More cache lines touched
* Indirect ownership semantics
* Historical allocator cache pollution

### Intrusive List

* Minimal node size (two pointers)
* No allocator involvement
* Explicit ownership
* Fewer instructions and stores
* More predictable cache behavior

This difference compounds at scale and directly affects **tail latency**.

---

## 6. Determinism & Tail Latency

Intrusive structures provide:

* No allocation jitter
* No deallocation stalls
* Stable memory layout
* Predictable instruction paths

In trading systems, these properties matter more than average throughput.

---

## 7. What This Benchmark Does *Not* Claim

* Intrusive lists are not universally better
* `std::list` is not “slow”
* Big-O complexity is unchanged
* This does not measure multi-threaded contention

The results support **engineering tradeoffs**, not blanket recommendations.

---

## 8. Real-World Relevance

These patterns appear in:

* LRU caches
* Order books
* Priority queues
* Time-ordered event lists
* Kernel schedulers
* HFT trading infrastructure

Intrusive structures are used where **control beats convenience**.

---

## 9. Next Steps

Planned extensions:

* LRU cache using both structures
* Skewed access patterns
* p50 / p99 / p999 latency histograms
* Cache-line footprint analysis
* Real-world access traces

---

## Summary

This experiment demonstrates that:

* O(1) does not mean equal cost
* Memory layout matters more than abstractions
* Ownership inversion reduces latency variance
* Intrusive structures are a deliberate design choice for low-latency systems

Understanding these tradeoffs is foundational for building reliable, high-performance trading infrastructure.
