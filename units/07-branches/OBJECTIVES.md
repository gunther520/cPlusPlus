# Unit 07 — Branches and prediction

## Target

- Measure a data-dependent `if` on **unsorted** versus **sorted** data (the classic mispredict demo).
- Compare a branchy loop with a **branchless** form (`cond ? x : y` / bit masks).
- Read p99: a mispredict is a pipeline flush; tails show it even when p50 looks “ok.”
- Optionally confirm with `perf stat -e branches,branch-misses`.

## Predict

1. Counting bytes `> 128`: random order vs sorted. Which has fewer mispredicts?
2. After sorting, is the remaining cost mostly arithmetic, or still memory (Unit 03)?
3. Will a branchless version beat sorted-branchy, or only beat *unsorted*?

## Run

```bash
cd units
make run-07
```

Optional:

```bash
perf stat -e branches,branch-misses,branch-load-misses ./bin/07-branches
```

## What you should see

- Unsorted: the comparator is ~50/50; the predictor cannot win. Slow p50 and ugly p99.
- Sorted: long runs of `taken` then `not-taken`. The predictor is almost perfect. Often several times faster.
- Branchless (mask / `cmov`): no (or few) mispredicts. Usually close to sorted-branchy, sometimes slightly slower than a *well predicted* branch because you always do both sides’ setup.

Your CPU’s predictor is very good. The demo needs a hard-to-predict pattern, not a loop `i < n`.

## Why

- A mispredicted branch throws away in-flight work. Cost is tens of cycles, not one.
- Sorting does not make the *algorithm* better at counting; it makes the *machine* better at guessing.
- Compilers emit `cmov` or SIMD compares when they can. A visible `if` in C++ is not always a `jcc` in asm — check Godbolt if the gap is tiny.

## Wrong conclusions

- “Never use `if` in HFT.” Predictable branches (error paths, loop conditions) are cheap. Unpredictable ones on the tick are not.
- Sorting online to make a one-pass scan predictable — the sort can cost more than you save unless you reuse the order.
- Benchmarking with `kN` so small the branch cost is lost in call overhead.

## Exercise

`TODO(unit-07)`: change the threshold from `128` to `250` (almost always false) on unsorted data.

A rare-taken branch is predictable again. Unsorted should speed up toward the sorted case. Then try `2` (almost always true). Prediction cares about *pattern*, not “sortedness” itself.
