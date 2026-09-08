# Unit 01 — Measure first

## Target

- Time the same work with a one-shot clock and with a warmed percentile bench.
- Read **p50 / p99 / p99.9**, not only the mean.
- See why `rdtsc` and `std::chrono::steady_clock` disagree, and when each is useful.
- Use `ll::do_not_optimize` so the compiler cannot delete the timed work.

## Predict

Before you run anything:

1. Will eight one-shot timings of the same loop agree within a few percent, or scatter?
2. After warmup, will p99 be close to p50, or much larger?
3. Is the first measurement usually the slowest? Why?

Write your guesses in a notebook, then run.

## Run

```bash
cd units
make run-01
make run-01 OPT=0    # optional: noise looks different without the optimizer
```

Pin to one core if the machine is busy:

```bash
taskset -c 0 make run-01
```

## What you should see

- One-shot lines **may** jump around. The first is often an outlier (cold I-cache, page faults, frequency boost). On a quiet isolated core they can look almost equal — that is not a failure. Read Case B.
- The percentile table is more stable across runs on an idle core. **p99 is higher than p50**; that gap *is* the latency story.
- `rdtsc` (calibrated to ns) tracks `steady_clock` on the same work, but is not a legal wall-clock. It can go backwards across cores and is not a `clock_gettime` replacement.

Absolute nanoseconds will differ on your CPU. Compare *shape* (scatter, p99/p50), not the exact number.

## Why

- A single `now()` delta includes everything that happened in that window: interrupts, migration, turbo, cold caches.
- Warmup pays the one-time costs so later samples describe the hot path.
- Sorting samples into percentiles answers “how bad is the tail?”, which is what “low latency” means.
- `rdtsc` reads a CPU cycle counter. Converted to ns it is handy inside a process. It is not a monotonic OS clock.

## Wrong conclusions

- “The function takes X ns” from one print of `clock_gettime`.
- Averaging one-shot runs and calling that p50. Mean hides the tail; a 1% stall dominates mean more than p50.
- Comparing `rdtsc` on core 0 with `chrono` on core 3 and declaring one clock “wrong.”
- Benchmarking at `-O0` and believing the numbers will hold in production (`-O2`).

## Exercise

In `compare.cpp`, find `TODO(unit-01)`. Change the inner trip count (`kWork`) by 10x and re-run.

- Does p50 scale ~linearly?
- Does p99/p50 stay similar, or does a fixed interrupt cost start to dominate at small `kWork`?

That ratio tells you when a microbench is measuring your code versus measuring the timer itself.
