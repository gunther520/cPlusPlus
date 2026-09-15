# Low-Latency C++ Tutorial

A unit-based course for people who already write C++ (classes, templates, STL) and want to
**measure and shape latency** on Linux: cache, allocations, branches, dispatch, atomics, SIMD,
syscalls, wait policy, and a small SPSC ring — plus a **capstone matcher** you implement yourself.

Each unit is a folder with:

- `OBJECTIVES.md` — what to learn, what to predict, how to run, why it works, how people misread the numbers, and an exercise
- `compare.cpp` — **Case A vs Case B** (sometimes C/D) in one binary, same data, same flags, p50/p99 printed

There is no Boost and no Google Benchmark. A small harness in [`units/common/bench.hpp`](units/common/bench.hpp) is the whole timing library.

## How to learn (do this every unit)

```text
read OBJECTIVES  →  write a prediction  →  run the binary  →  read Why  →  do the TODO
```

1. Read **Target** and **Predict**. Write guesses *before* you run. A number you predicted sticks.
2. Run the unit (commands below). Look at **p50 and p99**, not only the mean. Mean hides stalls.
3. Read **What you should see** and **Why**. If your machine disagrees in *shape* (who won), that is a bug in the lesson or a too-noisy box — pin a core and retry.
4. Do the **Exercise** (`TODO(unit-XX)` in `compare.cpp`). Re-run. That is the actual practice.

Keep a numbers log so later-you can compare machines:

| Unit | OPT | Case A p50 | Case A p99 | Case B p50 | Case B p99 | Notes |
|------|-----|------------|------------|------------|------------|-------|
| 01   | 2   |            |            |            |            |       |
| 03   | 2   |            |            |            |            |       |

Absolute nanoseconds are not portable. **Ratios and percentile shape** are.

## Build and run

Requires `g++` with C++17 and pthread (a normal Linux toolchain).

```bash
cd units
make help
make                 # all units, -O2
make run-01          # build + run unit 01
make run-02 OPT=0    # same source, no optimizer
make run-02 OPT=3
make clean
```

From the repo root: `make run-03`, `make run-02 OPT=0`, etc.

Unit 02 and 08 **must** be compared at `-O0` and `-O2`. Run `make clean` between `OPT` values; the output name does not include the level.

Thread units (04, 10, 12, 14) link `-pthread`. Capstone: `make -C capstone help`.

## Unit map

| Unit | Folder | Comparison |
|------|--------|------------|
| 01 | [units/01-measure-first](units/01-measure-first) | One-shot clock vs warmed p50/p99; chrono vs rdtsc |
| 02 | [units/02-compiler-hot-path](units/02-compiler-hot-path) | `-O0` vs `-O2`; dead-code elimination; inlining |
| 03 | [units/03-cache-locality](units/03-cache-locality) | Row vs column walk; AoS vs SoA |
| 04 | [units/04-false-sharing](units/04-false-sharing) | Adjacent atomics vs `alignas(64)` |
| 05 | [units/05-allocations](units/05-allocations) | `push_back` without `reserve`; `new` vs pool |
| 06 | [units/06-copies-and-strings](units/06-copies-and-strings) | `string` by value vs `string_view`; SSO; `reserve` |
| 07 | [units/07-branches](units/07-branches) | Unsorted vs sorted predicate; branchless |
| 08 | [units/08-virtual-vs-static](units/08-virtual-vs-static) | Virtual vs CRTP vs `std::function` |
| 09 | [units/09-data-oriented](units/09-data-oriented) | Linked list vs `vector` of values |
| 10 | [units/10-atomics-and-locks](units/10-atomics-and-locks) | Mutex vs atomic; relaxed vs seq_cst; acq/rel |
| 11 | [units/11-vectorization](units/11-vectorization) | Aliasing / branches vs restrict + contiguous SIMD |
| 12 | [units/12-capstone-spsc-loop](units/12-capstone-spsc-loop) | Locked `queue` vs SPSC ring buffer |
| 13 | [units/13-syscalls-hot-path](units/13-syscalls-hot-path) | Unbuffered `write` vs in-memory log vs no I/O |
| 14 | [units/14-spin-vs-sleep](units/14-spin-vs-sleep) | Busy spin vs `yield` vs `sleep_for` |
| 15 | [units/15-simd-intrinsics](units/15-simd-intrinsics) | Scalar add vs SSE2 `_mm_add_ps` vs auto-vec |

**Capstone (assignment):** [capstone/ASSIGNMENT.md](capstone/ASSIGNMENT.md) — mini price-time matcher. Run `make -C capstone run-naive`, then edit `capstone/starter/engine.hpp`.

Do the numbered units in order. Later units reuse *cache line*, *p99*, *do_not_optimize*, and *allocate before the event*. The capstone is where they have to show up in one program.

## Optional Linux tools

Not required. Useful when you want the hardware’s opinion, not only wall time.

```bash
# pin the process (and child threads of a make recipe: pin the binary)
taskset -c 0 ./units/bin/01-measure-first
taskset -c 0,1 ./units/bin/04-false-sharing

# cache and branches
perf stat -e cache-misses,cache-references,branches,branch-misses \
  ./units/bin/03-cache-locality
```

Compiler Explorer (https://godbolt.org/) with gcc `-O2` / `-O3 -march=native` is the fastest way to see whether Unit 02 inlined, Unit 08 stayed virtual, or Unit 11 emitted SIMD.

## Rules of thumb this course is training

- Measure the hot path at **`-O2` or `-O3`**, with warmup, with the result kept live.
- Optimize **layout and allocation** before micro-opcode twiddling.
- Predictable branches are cheap; unpredictable ones are not “never use `if`.”
- `relaxed` is for counters. **Acquire/release is for messages.** Dropping that for speed is a bug.
- One producer and one consumer can share a ring. Two producers cannot use the Unit 12 ring as-is.

## Out of scope (on purpose)

Kernel bypass (DPDK), huge pages, NUMA pinning, and a production multi-instrument matching engine. Units 13–15 and the capstone are the next rung, not the last.

## License / intent

Educational examples. They are deliberately small and sometimes “unfair” in the way real systems are (two cores, a full cache, a noisy VM). Trust the comparison in one process, one binary, one `OPT`.
