# Unit 19 lab (~1 hour)

After `make run-19`.

## Goal

Pay **page faults at startup**, not in the timed walk. Then see that **page stride** is a TLB loop, not Unit 03’s cache-line story.

## Run

```bash
cd units
make clean && make run-assign-19 OPT=2
```

Optional: `perf stat -e page-faults,dTLB-load-misses ./bin/assign-19`

## Tasks

1. Case A: `mmap`, first page-stride walk inside the timer (already). Record p50.
2. Case B: `mmap` and **prefault** (`memset` or a store to every page) **before** `steady_clock::now()`, then the same walk. Do not time the `memset`.
3. Case C: on a resident `vector`, walk with stride 16 (already). Case D: same touch count, stride = `PAGE_SIZE / sizeof(uint32_t)` (1024 on 4 KiB pages).
4. Optional: `madvise(..., MADV_HUGEPAGE)` after a prefault and compare to Case D. `MAP_HUGETLB` will often fail unless the host reserved huge pages.

## Done when

- Prefaulted walk beats first-touch p50 by a large factor (often tens of ×).
- Page-stride p50 is worse than cache-line stride at the same load count.
- You did not `munmap` before the walk (that is a use-after-free, not a cold start).

## Notes

| case | p50 | p99 |
|------|-----|-----|
| A first-touch |  |  |
| B prefault then walk |  |  |
| C stride 16 |  |  |
| D page stride |  |  |
