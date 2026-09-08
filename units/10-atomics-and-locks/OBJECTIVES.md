# Unit 10 — Atomics and locks

## Target

- Time a **mutex** around a shared counter versus `std::atomic` `fetch_add`.
- Contrast `memory_order_relaxed` with `acq_rel` / `seq_cst` on that counter.
- Put **correctness first**: the point is not “atomics always win,” it is “match the memory order to the publication you need.”
- Leave data races as a non-example (they are undefined behavior, not a fast path).

## Predict

1. Two threads incrementing the same location: mutex vs atomic relaxed — who has better p50?
2. Will `seq_cst` be slower than `relaxed` on x86? (Often a little; x86 TSO already has strong stores.)
3. Uncontended mutex in a *single* thread: still slower than a plain `++` or a relaxed atomic? Why?

## Run

```bash
cd units
make run-10
```

## What you should see

- Contended mutex: kernel/user futex, context switches possible, fat p99.
- Contended atomic: stays in user space, still *true sharing* of one line (Unit 04). Faster than the lock, not free.
- `relaxed` vs `seq_cst` on a pure counter: modest gap on x86; larger on weaker ISAs (ARM). This tutorial box is x86_64.
- Uncontended mutex (Case D) still pays lock/unlock; a relaxed atomic (or a non-shared `int` on a single thread) is cheaper.

## Why

- A mutex is a correctness tool for *regions*. If the critical section is one add, you paid a building to move a brick.
- `relaxed` only requires atomicity of that location. It does **not** publish other writes. Fine for a counter; wrong for “store payload then set ready flag.”
- `release` on the publisher + `acquire` on the reader is the usual “here is a message” pair. `seq_cst` is a total order — simplest to reason about, sometimes extra fences.

x86 `fetch_add` is a `lock xadd` regardless of C++ order (the RMW is always locked). Order still matters for *surrounding* loads/stores.

## Wrong conclusions

- `memory_order_relaxed` everywhere “for speed.” That is how you ship a Heisenbug.
- Replacing a mutex around a 200-line invariant with a single atomic flag and hoping.
- Benchmarking atomics without two threads and declaring locks “fine.”

## Exercise

`TODO(unit-10)`: in the publisher/consumer pair at the bottom, switch the flag stores from `release`/`acquire` to `relaxed`.

Then raise `kIters` and, if you can, run under ThreadSanitizer:

```bash
g++ -std=c++17 -O1 -g -fsanitize=thread -Icommon \
  10-atomics-and-locks/compare.cpp -pthread -o /tmp/u10-tsan
/tmp/u10-tsan
```

You may not always see a failure (x86 is strong), but the C++ model no longer guarantees the payload is visible. That is the lesson: **latency tricks that drop acquire/release are bugs, not optimizations.**
