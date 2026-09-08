# Unit 04 — False sharing

## Target

- See two threads fight over **one cache line** even when they write different variables.
- Fix it with `alignas(64)` so each counter owns a line.
- Connect this to MESI: a store invalidates the line in the other core’s cache.
- Know that this bug is invisible in a single-thread bench.

## Predict

1. Case A (`int a; int b;` adjacent) versus Case B (padded): which p50 is worse under two threads?
2. If you run only one thread, will they look the same?
3. Is this “lock contention”? (No. There is no lock.)

## Run

```bash
cd units
make run-04
```

Optional:

```bash
perf stat -e cache-misses,cache-references ./bin/04-false-sharing
taskset -c 0,1 ./bin/04-false-sharing
```

If the box has one core, the gap will be small — false sharing needs two cores bouncing a line.

## What you should see

- Two threads incrementing adjacent `int`s: much slower, noisy p99.
- Padded / `alignas(64)` counters: each thread’s stores stay local; p50 drops a lot.
- Single-thread control (Case C) is fastest of all and similar for padded vs not — the extra padding is not “free speed,” it only helps *sharing*.

## Why

- x86 cache lines are 64 bytes. `sizeof(int) == 4`, so `a` and `b` sit on the same line.
- Each increment is a read-modify-write of that line. The other core must invalidate and re-fetch. That is **false sharing**: logically independent data, physically one line.
- `alignas(64)` (or a `char pad[64 - sizeof(counter)]`) gives each field its own line.

## Wrong conclusions

- “Atomics are slow” when you put two `std::atomic<int>` neighbors in one struct and hammer them from two threads.
- Padding every field in a single-thread struct “for performance.” You just bloat the working set (Unit 03).
- Measuring with both threads pinned to the **same** core — they cannot bounce a line if they never run together.

## Exercise

`TODO(unit-04)`: change `Padded` so the pad is `alignas(32)` (half a line) instead of 64.

- Do the two counters still share a line? Re-run.
- Then try `alignas(64)` again. The experiment is the definition of a cache line.
