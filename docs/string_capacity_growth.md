# **string_capacity_growth.md**

## **String Capacity Growth Analysis (libstdc++ / GCC / Linux)**

This document summarizes the observed capacity growth pattern of `std::string` when characters are appended one-by-one. The measurements were collected by printing:

* current length (`i`)
* current `size()`
* current `capacity()`
* the memory address of `&s[0]`

after each `push_back()`.

---

## **1. Small String Optimization (SSO) Region: Length 0–15**

For lengths **0 to 15**, the string remains inside the SSO buffer:

* `capacity = 15`
* `data()` points to the **same stack/local buffer**
* no heap allocation
* no reallocations

This confirms the expected **15-byte SSO threshold** used by libstdc++.

---

## **2. First Heap Allocation: Length 16**

At length = 16:

* SSO is exhausted
* String allocates heap memory
* New capacity becomes **30**
* The address of `data()` jumps to the heap

This is the first dynamic allocation.

---

## **3. Growth Pattern Observed**

After exiting SSO, the capacity grows roughly with the following multipliers:

* From 15 → **30**  (2×)
* From 30 → **60**  (2×)
* From 60 → **120** (2×)
* From 120 → **240**
* From 240 → **480**
* From 480 → **960**

This matches libstdc++’s growth rule:

**New capacity = old_capacity × 2**

until the implementation reaches its internal thresholds where different policies may apply.

---

## **4. Number of Reallocations Observed**

Reallocations happen at the exact breakpoints:

* 15 → 16 (SSO → heap)
* 30 → 31
* 60 → 61
* 120 → 121
* 240 → 241
* 480 → 481
* 960 → 961 (if continued)

Each reallocation moves every character in the string, which becomes expensive for large sizes.

---

## **5. Memory Address Changes**

Within SSO:

* Address remains constant: on-stack buffer

After SSO:

* Address changes at every reallocation event
* Between reallocations the address stays the same

This demonstrates that:

* Iterators & pointers remain stable **until growth triggers a reallocation**
* After reallocation **all references become invalid**

---

## **6. Practical Takeaways for Performance**

### ✔ Reserving capacity avoids expensive reallocation

If you know a string will reach N characters, always call:

```
s.reserve(N);
```

This results in:

* fewer reallocations
* reduced copying cost
* far more predictable latency (critical for HFT, low-latency C++)

### ✔ SSO is extremely fast

Operations inside the SSO region:

* require **zero allocations**
* are very cache friendly
* are ideal for small tokens / short messages

### ✔ Growth factor ≈ 2×

This amortizes insertion cost to **O(1)** while still limiting memory waste.

---

## **7. Summary**

The experiment confirms all major aspects of libstdc++ string behavior:

* **SSO up to 15 bytes**
* **Heap transition at size 16**
* **Doubling growth strategy**
* **Reallocation every time the size crosses a power-of-two threshold**
* **Iterator invalidation on reallocation**
* **Consistent capacity jumps: 15 → 30 → 60 → 120 → 240 → 480 → 960**

This knowledge is foundational for writing low-latency, high-performance string-heavy systems in C++.
