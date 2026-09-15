# Unit 11 lab (~1 hour)

After `make run-11 OPT=3`.

## Goal

Feel auto-vectorization: a loop gcc **will not** vectorize well vs one it will. Inspect asm once.

## Run

```bash
cd units
make clean && make run-assign-11 OPT=3
g++ -std=c++17 -O3 -Icommon -S 11-vectorization/assignment.cpp -o /tmp/u11a.s
```

## Tasks

1. Case A: branchy masked sum (`if (mask[i]) s += a[i]`).
2. Case B: branchless `s += a[i] * mask[i]`.
3. Optional: `float` add with `__restrict__` vs a `noinline` aliasing signature (see compare.cpp).
4. In `/tmp/u11a.s` search for `addps`/`mulps`/`xmm`. Did Case B get SIMD?

## Done when

- Branchless p50 beats branchy (often several ×).
- You noted whether you saw SIMD in the asm (yes/no is enough).
- You did not enable `-ffast-math` to “cheat” a reduction unless you say so in notes.

## Notes

| case | p50 | SIMD in asm? |
|------|-----|----------------|
| branchy |  |  |
| branchless |  |  |
