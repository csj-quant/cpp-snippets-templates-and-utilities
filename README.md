# **cpp-snippets-templates-and-utilities**

A growing collection of reusable C++ utilities, diagnostics, template patterns, and performance-engineering tools.
This project serves as a central library of small, focused components that illustrate low-level C++ behavior, best practices, and micro-benchmarks across different areas of systems programming.

Over time, this repository will expand to cover templates, iterators, allocators, concurrency primitives, metaprogramming utilities, and other components from the broader development roadmap.

---

# **📦 Current Module: String Internals & Performance Diagnostics (C++23)**

This module provides experimental tools for understanding low-level behavior of `std::string` in GCC/libstdc++.
The goal is to build intuition around **memory layout**, **SSO (small-string optimization)**, **capacity growth**, and **copy/move performance**—critical for designing predictable, low-latency C++ systems.

---

## **1. Small String Optimization (SSO) Inspector**

**File:** `src/string_sso_inspector.cpp`
This diagnostic prints the memory address, size, and capacity of strings from length 0 to 50.

### **Findings**

* libstdc++ uses a **15-byte SSO buffer**
* Strings of length ≤ 15 require **no heap allocation**
* At length 16, strings transition to heap storage
* After leaving SSO, capacity grows dynamically

This reveals how efficiently small strings are stored and when allocations begin.

---

## **2. Copy vs Move Benchmark**

**File:** `src/string_copy_move_bench.cpp`
Benchmarks the cost of copying vs moving **1,000 strings of size 1,000,000**.

### **Results**

```
Copy 1000 strings  : ~1764 ms
Move 1000 strings  : ~0.0045 ms
```

### **Interpretation**

Copying requires:

* allocating new buffers
* copying 1 million bytes per string

Moving requires:

* swapping pointers
* zero data movement

Move operations are **~400,000× faster** than deep copies for large strings.

This demonstrates why modern C++ systems—especially low-latency pipelines—rely heavily on move semantics.

---

## **3. String Capacity Growth Diagnostics**

**File:** `src/string_capacity_growth.cpp`
Pushes characters one at a time and logs:

* size
* capacity
* memory address

### **Observed Growth Pattern**

```
SSO capacity     : 15
First heap alloc : 30
Next expansions  : 60 → 120 → 240 → 480 → 960 ...
```

Growth approximately doubles each time (`capacity *= 2`).
This ensures amortized **O(1)** push_back, but also means:

* pointers/iterators are invalidated on reallocation
* allocations become progressively more expensive
* predictable capacity usage matters in high-performance systems

---

## **4. Documentation**

Located in the `docs/` directory:

* **string_sso_notes.md**
  Summary of SSO behavior and threshold discovery.

* **string_copy_move_analysis.md**
  Explanation of measured timings and the mechanics behind copy vs move.

* **string_capacity_growth.md**
  Capacity expansion pattern with notes on reallocation behavior.

---

# **🧱 Repository Structure**

```
cpp-snippets-templates-and-utilities/
│
├── src/
│   ├── string_sso_inspector.cpp
│   ├── string_copy_move_bench.cpp
│   ├── string_capacity_growth.cpp
│
└── docs/
    ├── string_sso_notes.md
    ├── string_copy_move_analysis.md
    └── string_capacity_growth.md
```

This structure will grow as more modules are added.

---

# **🚀 Planned Additions (Reserved Structure)**

These sections will be filled as new utilities are added throughout the roadmap:

### **🔸 Iterator & Pointer Semantics Module**

* iterator categories
* contiguous vs non-contiguous storage
* aliasing diagnostics
* iterator invalidation tools

### **🔸 Template Utilities Module**

* CRTP patterns
* type traits & SFINAE helpers
* constexpr utilities

### **🔸 Concurrency & Atomics Module**

* lock-free structures
* cache-line alignment tools
* false-sharing detectors

### **🔸 Allocators & Memory Resource Module**

* custom allocators
* memory pools
* profiling allocation behavior

### **🔸 Snippets for Competitive Programming / System Utilities**

* reusable algorithm templates
* high-performance data structures
* string/array/graph helper functions

Each module will follow the same “experiment + benchmark + documentation” pattern as the string internals.

---

# **📌 Build Instructions**

```
mkdir build
cd build
cmake ..
make
```

Executables will appear in the build directory.

---

# **📄 License**

This project is maintained as an open, evolving toolbox for systems-level C++ learning, performance research, and rapid experimentation.
