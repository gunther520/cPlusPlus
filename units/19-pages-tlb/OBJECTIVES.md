# Unit 19 — Pages, TLB, and prefault

## Target

- See **minor page faults** as a hot-path tax: anonymous `mmap` is not RAM until you touch it (Unit 01’s cold start, named).
- **Prefault** (touch or `MAP_POPULATE`) at startup so the tick does not pay the kernel.
- Distinguish **cache-line stride** (Unit 03) from **page stride** (TLB). The TLB is a cache of virtual→physical translations; a 4 KiB page walk misses it far more often than a sequential scan.
- Know that **2 MiB huge pages** cut TLB pressure, but `MAP_HUGETLB` needs reserved pages. `MADV_HUGEPAGE` (THP) is optional and can jitter p99 in production — measure, do not assume.

## Predict

1. 64 MiB, one access per 4 KiB page, first touch inside the timer vs the same walk after a startup `memset`. Who owns p50?
2. Resident array, same number of loads: stride 16 `uint32_t` (one cache line) vs stride 1024 (one page). Is that a cache story or a TLB story?
3. Will `__builtin_prefetch` on the page-stride walk help, or is the hardware prefetcher already trying?

## Run

```bash
cd units
make run-19
```

Optional:

```bash
perf stat -e dTLB-load-misses,page-faults,cache-misses ./bin/19-pages-tlb
```

## What you should see

- Case A (cold first-touch) many times slower than Case B (prefault, then walk). The gap is faults, not arithmetic.
- Case C (cache-line stride) faster than Case D (page stride) on a large resident buffer. Same load count; worse translation locality.
- `MAP_HUGETLB` often **fails** on a VM with `nr_hugepages=0`. Case E then uses `MADV_HUGEPAGE` after a prefault; it may be a modest win or a wash. That is the point: huge pages are a **kernel reservation**, not a C++ keyword.

## Why

- The first store to a new anonymous page is a minor fault (kernel fills a frame, updates the page table). Thousands of those in a tick blow p99.
- The dTLB holds a handful of page translations (often hundreds, not millions). Stride-by-page is a TLB-miss loop. Sequential stride-by-line reuses one translation for 64 loads.
- Huge pages (2 MiB) mean one TLB entry covers 512× more bytes. HFT boxes **reserve** them and often **disable** transparent huge pages, because `khugepaged` compaction is a tail-latency surprise.

## Wrong conclusions

- “`mmap` returns memory I can use at L1 speed.” It returns **address space**.
- Prefaulting in the timed region and calling that the hot path. Prefault belongs in **startup** (or a quiet phase), like `reserve` (Unit 05).
- Enabling THP system-wide because Case E got 20% better p50. Ask p99 over hours, not one microbench.
- Prefetch as a default. On a streaming page-stride loop the hardware already prefetches; extra `__builtin_prefetch` often does nothing or steals issue slots.

## Exercise

`TODO(unit-19)`: in Case D, prefetch ~8 pages ahead (`__builtin_prefetch(p + i + 8 * stride)`). Re-run.

If p50 drops, you were stalling on translation/data. If it does not, you were not bound on that miss — stop decorating the loop.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-19`).
