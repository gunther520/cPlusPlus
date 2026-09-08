# Unit 01 — Measure first

## Target

- Time the same work with a one-shot clock and with a warmed percentile bench.
- Read **p50 / p99 / p99.9**, not only the mean.
- See why `rdtsc` and `std::chrono::steady_clock` disagree, and when each is useful.
- Use `ll::do_not_optimize` so the compiler cannot delete the timed work.

## Constants in `compare.cpp`

None of these are "the latency of a real system." They only size the demo.

| Name | Value | Why that number |
|------|-------|-----------------|
| `kWork` | 250000 | One `burn` call lasts ~0.1–1 ms so you are timing the loop, not the clock. |
| `kOneShotRuns` | 8 | Enough Case A lines to see scatter without flooding the terminal. |
| `kSamples` | 200 | Need a pile of samples or "p99" is just the single slowest run. |
| `kWarmup` | 20 | Untimed hot-path calls before Case B. Useless if `data` is already resident (the fill loop did that). |
| `kMix64` / `kMix32` | `0x9e3779b9…` | Traditional golden-ratio hash bits (`2^n / φ`). Ugly on purpose so `-O2` cannot algebraically delete the loop. |
| `kLcgMul` / `kLcgAdd` | 1103515245, 12345 | Classic `rand()` LCG. Fills the array with a non-linear pattern. |
| `<< 6`, `>> 2` | — | Mix high and low bits of `sum` each iteration (not a "real" hash). |

`ll::do_not_optimize(sum)` is not a number: it tells the compiler "this value escapes," so the loop must actually run.

## Predict

Before you run anything:

1. After the fill loop, will three extra `burn` calls get faster? Or are they already hot?
2. A brand-new `mmap` whose pages have never been touched: will the first `burn` be much slower than the hot runs?
3. After warmup on the filled buffer, will p99 be close to p50, or still larger?

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

- **Already-hot lines** (right after fill) should look like each other. The fill loop *was* the warmup. Calling `ll::bench(..., warmup=20)` on top of that will not move the number much.
- **Case A (fresh mmap, first touch)** should be clearly slower: the timed region includes minor page faults. That is a real cold start.
- **Case B p50** should match the hot lines. That is what warmup is for: measure the steady path, not the first fault.
- **p99 is still higher than p50** on Case B. Warmup does not remove tail latency; it only skips the *one-time* cold start.
- `rdtsc` (calibrated to ns) tracks `steady_clock` on the same hot work, but is not a legal wall-clock.

Absolute nanoseconds will differ on your CPU. Compare *shape* (scatter, p99/p50), not the exact number.

## Why

- Warmup pays one-time costs **when they have not already been paid**. Filling `data` already faulted pages, so a later `kWarmup` loop looks like a no-op. Case A’s fresh `mmap` puts those faults back *inside* the timed region so you can see them.
- A single `now()` delta includes everything in that window: faults, interrupts, migration, turbo.
- Sorting samples into percentiles answers “how bad is the tail?”, which is what “low latency” means. Warmup does not flatten p99.
- `rdtsc` reads a CPU cycle counter. Converted to ns it is handy inside a process. It is not a monotonic OS clock.

## Wrong conclusions

- “Warmup never matters” because this file’s fill loop already warmed the array. That is a property of the *setup*, not a law of CPUs.
- “The function takes X ns” from one print of `clock_gettime`.
- Averaging one-shot runs and calling that p50. Mean hides the tail; a 1% stall dominates mean more than p50.
- Comparing `rdtsc` on core 0 with `chrono` on core 3 and declaring one clock “wrong.”
- Benchmarking at `-O0` and believing the numbers will hold in production (`-O2`).

## Exercise

In `compare.cpp`, find `TODO(unit-01)`. Change the inner trip count (`kWork`) by 10x and re-run.

- Does p50 scale ~linearly?
- Does p99/p50 stay similar, or does a fixed interrupt cost start to dominate at small `kWork`?

That ratio tells you when a microbench is measuring your code versus measuring the timer itself.
