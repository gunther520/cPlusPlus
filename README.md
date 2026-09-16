# Low-Latency C++ Tutorial

A unit-based course for people who already write C++ (classes, templates, STL) and want to
**measure and shape latency** on Linux: cache, allocations, branches, dispatch, atomics, SIMD,
syscalls, wait policy, a small SPSC ring, runtime CPU/cache dispatch, **pages/TLB**, **exceptions**, and a **measured latency budget** — plus a **capstone matcher** you implement yourself.

Each unit is a folder with:

- `OBJECTIVES.md` — what to learn, what to predict, how to run, why it works, how people misread the numbers
- `compare.cpp` — **Case A vs Case B** in one binary, p50/p99 printed
- `ASSIGNMENT.md` + `assignment.cpp` — a **~1 hour lab** (starter compiles; Case B is yours)

There is no Boost and no Google Benchmark. A small harness in [`units/common/bench.hpp`](units/common/bench.hpp) is the whole timing library.
Shared SPSC: [`units/common/spsc.hpp`](units/common/spsc.hpp). **Core vs optional:** [`TRACK.md`](TRACK.md). **Pipeline sketch:** [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## How to learn (do this every unit)

```text
read OBJECTIVES  →  run compare  →  read Why  →  1-hour lab (ASSIGNMENT.md)
```

1. Read **Target** and **Predict**. Write guesses *before* you run.
2. `make run-NN` and look at **p50 and p99**, not only the mean.
3. Read **What you should see** and **Why**.
4. Do the **~1 hour lab**: `make run-assign-NN`, edit `assignment.cpp`, fill the notes table in `ASSIGNMENT.md`.

Keep a numbers log so later-you can compare machines:

| Unit | OPT | Case A p50 | Case A p99 | Case B p50 | Case B p99 | Notes |
|------|-----|------------|------------|------------|------------|-------|
| 01   | 2   |            |            |            |            |       |
| 03   | 2   |            |            |            |            |       |

Absolute nanoseconds are not portable. **Ratios and percentile shape** are.

## Build and run

Requires **g++** (tested 13.x) with C++17 and pthread on **Linux x86-64**. GCC attributes (`noinline`, `target("avx2")`, Itanium EH) are the course ABI, not portable ISO C++.

```bash
cd units
make help
make                 # all units, -O2 → bin/O2/
make run-01          # build and run one lesson
make run-02 OPT=0    # same source, no optimizer (separate bin/O0/)
make run-assign-01   # ~1h lab for unit 01
make assignments     # build every lab starter
make check           # Unit 12 payload oracle
```

From the repo root: `make run-03`, `make check` (units + capstone), `make tsan`.

Binaries land in `units/bin/O$(OPT)/` (and `capstone/bin/O$(OPT)/`), so **you do not need `make clean` to switch OPT**. Thread units pin with `taskset -c 0,1` when `PIN=1` (default). `PIN=0` to disable.

Each run prints a `csv,unit,case,p50_ns,p99_ns,n` line (`tee -a numbers.csv`). If `n<50`, p99 is tagged `~max` — that is the worst sample, not a real percentile.

Thread units (04, 10, 12, 14, 16) link `-pthread`. Unit 17 uses `fork` (no extra `-pthread`). Capstone: `make -C capstone help`.

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
| 12 | [units/12-spsc-ring](units/12-spsc-ring) | Unbounded mutex queue vs locked ring vs SPSC |
| 13 | [units/13-syscalls-hot-path](units/13-syscalls-hot-path) | Unbuffered `write` vs in-memory log vs no I/O |
| 14 | [units/14-spin-vs-sleep](units/14-spin-vs-sleep) | Busy spin vs `yield` vs `sleep_for` |
| 15 | [units/15-simd-intrinsics](units/15-simd-intrinsics) | Scalar add vs SSE2 `_mm_add_ps` vs auto-vec |
| 16 | [units/16-shard-per-core](units/16-shard-per-core) | Mutex map vs shard-per-core (parallel without sharing) |
| 17 | [units/17-ipc-shared-memory](units/17-ipc-shared-memory) | Unix `socketpair` vs mmap SPSC (two processes) |
| 18 | [units/18-hw-adaptive](units/18-hw-adaptive) | Runtime ISA pick (SSE2/AVX2) vs scalar; L1-sized tiling |
| 19 | [units/19-pages-tlb](units/19-pages-tlb) | First-touch vs prefault; cache-line vs page stride (TLB); THP |
| 20 | [units/20-exceptions](units/20-exceptions) | Error codes vs `throw` (0% vs 1% fail) |
| 21 | [units/21-latency-budget](units/21-latency-budget) | Dependent chase L1 vs LLC; hops that fit in 500 ns |

Each numbered unit has a lab: `make run-assign-NN` (see that folder’s `ASSIGNMENT.md`).

**Capstone (multi-hour project):** [capstone/ASSIGNMENT.md](capstone/ASSIGNMENT.md) — mini price-time matcher after the units.

## Optional Linux tools

Not required. Useful when you want the hardware’s opinion, not only wall time.

```bash
# pin the process (and child threads of a make recipe: pin the binary)
taskset -c 0 ./units/bin/O2/01-measure-first
taskset -c 0,1 ./units/bin/O2/04-false-sharing

# cache and branches (or: make -C units perf-03)
perf stat -e cache-misses,cache-references,branches,branch-misses \
  ./units/bin/O2/03-cache-locality
```

Or: `make -C units perf-03`

Compiler Explorer (https://godbolt.org/) with gcc `-O2` / `-O3 -march=native` is the fastest way to see whether Unit 02 inlined, Unit 08 stayed virtual, or Unit 11 emitted SIMD.

## Rules of thumb this course is training

- Measure the hot path at **`-O2` or `-O3`**, with warmup, with the result kept live.
- Optimize **layout and allocation** before micro-opcode twiddling.
- Predictable branches are cheap; unpredictable ones are not “never use `if`.”
- `relaxed` is for counters. **Acquire/release is for messages.** Dropping that for speed is a bug.
- One producer and one consumer can share a ring. Two producers cannot use the Unit 12 ring as-is.
- Parallelism for latency is **sharding / pipelines**, not a bigger mutex (Unit 16).
- Two processes on one box: prefer shared memory over sockets (Unit 17). A network hop is a different budget.
- One binary, many CPUs: pick the kernel at **startup** (`__builtin_cpu_supports`, L1 from sysfs), not with `-march=native` (Unit 18).
- Anonymous `mmap` is address space. **Prefault** (and huge pages if you reserved them) before the tick (Unit 19).
- Do not `throw` on the expected path. Return a code; unwind is p99 (Unit 20).
- Budget **dependent** misses, not sequential SIMD adds (Unit 21).

## Out of scope (on purpose)

Kernel bypass (DPDK), NUMA pinning, MPI/Spark clusters, and a production multi-instrument matching engine. Units 16–17 are intra-host parallel/IPC, not wide-area distributed systems. Unit 18 is startup dispatch, not a hot-path autotuner. Unit 19 demos THP/`MAP_HUGETLB` when the kernel allows it — it does not reserve huge pages for you.

## License / intent

Educational examples. They are deliberately small and sometimes “unfair” in the way real systems are (two cores, a full cache, a noisy VM). Trust the comparison in one process, one binary, one `OPT`.
