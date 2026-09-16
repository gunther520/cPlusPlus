# Unit 12 lab (~1 hour)

After `make run-12`.

## Goal

Move ~200k ints from one producer to one consumer. Replace `mutex + queue` with **`SpscRing<int, 1024>`** from `common/spsc.hpp` (or copy the same protocol). The consumer sum is checked.

## Run

```bash
cd units
make run-assign-12
```

## Tasks

1. Case A: `LockedQueue` (already). Record p50/p99.
2. Case B: `StudentRing` should be `SpscRing<int, kCap>` (include `"spsc.hpp"`). `try_push` must not allocate.
3. Try `kCap = 8` vs `4096`.
4. Optional: a mutex + bounded ring (compare.cpp Case B) to see lock vs SPSC.

## Done when

- SPSC p50 beats locked queue.
- The binary does not print `CHECK FAILED` (sum of `0..n-1`).
- You did not use two producers.

## Notes

| cap | locked p50 | spsc p50 |
|-----|------------|----------|
| 1024 |  |  |
| 8    |  |  |
