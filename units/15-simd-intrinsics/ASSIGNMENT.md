# Unit 15 lab (~1 hour)

After `make run-15 OPT=2`.

## Goal

Write a **SSE2** add (or dot product) and beat a scalar loop that is not allowed to auto-vectorize. Stay on 128-bit (`__m128`).

## Run

```bash
cd units
make clean && make run-assign-15 OPT=2
```

x86_64 only for Case B. Scalar still runs everywhere.

## Tasks

1. Case A: `noinline` + `optimize("no-tree-vectorize")` scalar add (already).
2. Case B: `_mm_loadu_ps` / `_mm_add_ps` / `_mm_storeu_ps` in steps of 4, plus a scalar tail.
3. Keep `kN` modest (`1<<15`) so you are not purely DRAM-bound. Then try `1<<20` and see the win shrink.
4. Optional: 8-float unroll (two `__m128`).

## Done when

- SSE2 p50 beats scalar at `kN=1<<15` (often ~3×).
- Tail handles `n % 4 != 0` (test with `kN+1` once).
- You did not claim victory at `-O0`.

## Notes

| kN | scalar p50 | SSE2 p50 |
|----|------------|----------|
| 1<<15 |  |  |
| 1<<20 |  |  |
