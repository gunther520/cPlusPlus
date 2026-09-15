# Unit 18 lab (~1 hour)

After `make run-18 OPT=2`.

## Goal

Make **one binary** adapt to **this** machine:

1. Pick a Horner-polynomial kernel with `__builtin_cpu_supports` (AVX2 if legal, else SSE2, else scalar).
2. Parse **L1 data** cache size from sysfs and use it as the **column-strip width** for a blocked matrix sum.

Do not add `-march=native` or `-mavx2` to the make flags.

## Run

```bash
cd units
make clean && make run-assign-18 OPT=2
```

x86_64 for SIMD Case B. Sysfs tiling still applies on any Linux.

## Tasks

1. Case A: `noinline` scalar polynomial (already). Record p50.
2. Case B: implement `pick()`. Call `__builtin_cpu_init()` first. Return `poly_avx2` / `poly_sse2` / `poly_scalar`. The AVX2 function is already marked `target("avx2")` — do not paste `_mm256_*` into `main`.
3. Print what you picked. If you always return scalar, Case B will match A (that is the dispatcher, not a compiler bug).
4. Implement `read_l1d_bytes()`: walk `/sys/devices/system/cpu/cpu0/cache/index*/` until `type == Data` and `level == 1`, parse `size` (`48K` → bytes). Fallback `32 * 1024` if sysfs is missing.
5. Case C is a column-major walk (stride `kCols`). Case D should call `l1_col_tile(kRows)`: about half of L1d divided by `rows * sizeof(float)`, clamped to `[4, 64]` — not `3`.
6. Optional: if this CPU has AVX2, compare always-SSE2 vs `pick()` on `kN=1<<15`, then on a single `add_ps` of `1<<20` (the ISA gap should shrink).

## Done when

- `pick()` is not hard-coded to scalar on an AVX2 box (Case B beats A at `kN=1<<15`).
- L1 parse prints a plausible KiB (often tens of KiB per core, not L3’s hundreds of MiB).
- L1 column tile beats Case C, or you wrote why it did not (matrix already tiny; hypervisor cache sizes).
- You did not compile this lab with `-march=native`.

## Notes

| case | p50 | p99 | kernel / tile |
|------|-----|-----|----------------|
| A scalar poly |  |  | scalar |
| B pick() |  |  |  |
| C column-major |  |  | 1 |
| D L1 col tile |  |  |  |
