# Unit 07 lab (~1 hour)

After `make run-07`.

## Goal

Count “large” bytes two ways: branchy on **unsorted** data vs sorted vs a **branchless** form. Then change the threshold so the branch becomes predictable.

## Run

```bash
cd units
make run-assign-07
```

## Tasks

1. Fill 1M `uint8_t` random bytes. Count `> threshold` with a `noinline` hit (so gcc cannot cmov the whole thing away).
2. Repeat on a **sorted** copy. Record speedup.
3. Write a branchless count (`c += (p[i] > thr)`).
4. Set threshold to `2` and `250` on unsorted data. When does unsorted catch up?

## Done when

- Sorted branchy beats unsorted at threshold 128 (often >2×).
- You recorded what happens at threshold 2 and 250.
- You did not sort inside the timed loop of the “sorted” case (sort once in setup).

## Notes

| data | threshold | p50 |
|------|-----------|-----|
| unsorted branchy | 128 |  |
| sorted branchy   | 128 |  |
| branchless       | 128 |  |
| unsorted branchy | 2   |  |
| unsorted branchy | 250 |  |
