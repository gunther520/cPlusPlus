# Unit 05 — Allocations on the hot path

## Target

- See `std::vector::push_back` without `reserve` reallocate (copy + heap) as it grows.
- Compare **heap `new` per node** versus a **pre-sized pool**.
- Treat `malloc`/`new` as an unbounded-latency call (locks, syscalls, page faults).
- Practice the low-latency default: **allocate before the event, reuse after**.

## Predict

1. `push_back` N times: with `reserve(N)` vs without. Who wins p99, not just p50?
2. Is the gap mostly p50 (every insert a bit slower) or p99 (rare huge stalls)?
3. A pool of N nodes versus `new`/`delete` each sample: which tail is fatter?

## Run

```bash
cd units
make run-05
```

## What you should see

- Unreserved `push_back`: higher p50 *and* a nastier p99 (a sample that hit several growths / copies).
- `reserve`: one allocation, then stores into already-owned memory.
- `new` per node: allocator traffic on every sample. A bump/pool or `vector` of values: almost no heap in the timed region.

If p99 of Case A is only mildly worse, raise `kN` (exercise). Growth events are sparse; percentiles need enough samples to *catch* one.

## Why

- `vector` growth is geometric; each growth allocates, copies, frees. That is correct and still poison for a latency SLO.
- `new` may take a mutex in glibc, may unmap/mmap, may fault a fresh page (Unit 01’s “first sample is slow” at smaller scale).
- Pools, arenas, and `reserve` move allocation to startup (or a quiet phase). The hot path becomes pointer bump or indexed reuse.

## Wrong conclusions

- “Never use the heap.” Setup and cold paths can allocate. The **trading/audio/game tick** should not.
- Comparing a pool that is not sized for the burst, then blaming pools when they fall back to `new`.
- Forgetting destructor cost: `delete` in the hot path is also latency.

## Exercise

`TODO(unit-05)`: in Case A, call `v.shrink_to_fit()` after `clear()` (uncomment the line).

That *gives back* capacity so the next sample grows from empty again. p99 should get worse. Then replace it with `v.reserve(kN)` after `clear` and watch the tail calm down.
