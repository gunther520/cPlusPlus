# Unit 03 — Cache locality

## Target

- Walk the same matrix **row-major** versus **column-major** and see DRAM vs cache.
- Compare **array-of-structs (AoS)** versus **struct-of-arrays (SoA)** when you only need one field.
- Name a cache line (~64 bytes on this x86) and why stride matters.
- Optionally confirm the story with `perf stat` cache counters.

## Predict

1. For a `N x N` matrix stored row-major, is `a[row][col]` or `a[col][row]` faster? Why?
2. If you sum only `price` from millions of `Order` records, is AoS or SoA kinder to L1?
3. Will the gap shrink if `N` fits in L1? (Try the exercise.)

## Run

```bash
cd units
make run-03
```

Optional hardware counters (Linux):

```bash
perf stat -e cache-misses,cache-references,L1-dcache-load-misses \
  ./bin/03-cache-locality
```

Pinning helps:

```bash
taskset -c 0 make run-03
```

## What you should see

- Column-major walk of a row-major matrix is much slower (often several times) once `N*N` exceeds last-level cache.
- SoA sum of one field beats AoS: each cache line is full of `price`, not `id`/`qty`/`pad`.
- Absolute times depend on `N` and your cache sizes (`lscpu` / `/sys/devices/system/cpu/cpu0/cache/`).

## Why

- Hardware prefetches sequential (and some strided) streams. A column walk jumps `N * sizeof(int)` bytes — many cache lines, few useful bytes each.
- A 64-byte line holds 16 `int`s or 8 `double`s. AoS wastes line capacity on fields you do not touch.
- This is the core of data-oriented design (Unit 09 continues it with pointer chasing).

## Wrong conclusions

- “2D arrays are slow in C++.” Storage order vs access order is the bug.
- Microbenching `N=8` (fits in registers) and declaring locality a myth.
- Copying into SoA “for speed” on a one-shot pass that you will not reuse — the copy can cost more than the gain.

## Exercise

`TODO(unit-03)`: drop `kN` from 1024 toward 64, then up toward 2048.

- At what `N` does the row/column gap explode? That is roughly “working set left the cache.”
- At tiny `N`, both cases should look alike; you are measuring arithmetic, not memory.
