# Unit 04 lab (~1 hour)

After `make run-04`.

## Goal

Find false sharing in a two-thread counter pair and kill it with `alignas(64)`.

## Run

```bash
cd units
make run-assign-04
```

Need two cores. If Case A ≈ Case B, you may be on one core.

## Tasks

1. Case A: two `atomic<uint64_t>` neighbors, two threads incrementing one each.
2. Case B is the same layout — **pad or alignas(64)** so each counter owns a line.
3. Record sizeof both structs and p50. Optional: `perf stat -e cache-misses`.

## Done when

- `sizeof(Padded)` is ≥ 128 on this x86.
- Padded p50 is clearly faster than adjacent (several × if two cores).
- You can explain it without the word “lock” (there is no mutex).

## Notes

| struct | sizeof | p50 | p99 |
|--------|--------|-----|-----|
| adjacent |  |  |  |
| padded   |  |  |  |
