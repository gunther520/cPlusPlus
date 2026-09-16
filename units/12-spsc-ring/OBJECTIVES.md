# Unit 12 — SPSC ring vs locked queue

## Target

- Move hundreds of thousands of integers from one producer thread to one consumer.
- Split **heap** from **the lock** from **SPSC**: unbounded `mutex + queue`, mutex + **pre-sized** ring, then `SpscRing` from [`common/spsc.hpp`](../common/spsc.hpp).
- Reuse cache-line padding (Unit 04), acquire/release (Unit 10), no heap on the tick (Unit 05).
- Contract: **one** pusher, **one** popper. Two producers is a bug, not a faster queue.

## Predict

1. Unbounded locked queue vs locked bounded ring: who wins p50? (allocation vs lock)
2. Locked bounded ring vs SPSC: who wins? (the lock itself)
3. Tiny `kCap = 8`: does the producer stall so the unbounded queue looks better?

## Run

```bash
cd units
make run-12
taskset -c 0,1 ./bin/O2/12-spsc-ring   # after make; PIN is on by default for thread units
```

## What you should see

- Case A: mutex + heap growth as `queue` expands. Fat tails.
- Case B: mutex + fixed slots. Often faster than A (no `new`), still a lock.
- Case C: SPSC. Publication is write-slot then `release`; reader `acquire` then read-slot. Sum of payloads is checked (`0+…+n-1`).

## Why

- SPSC needs no lock because each index is owned by one thread.
- Comparing A to C mixes two variables. B exists so you can see the lock and the allocator separately.

## Wrong conclusions

- Two producers on this ring. ABA and lost updates. Use shards (Unit 16) or a real MPMC.
- “The ring is faster because it is lock-free” when you only compared to `std::queue` (heap).

## Exercise

`TODO(unit-12)`: try `kCap = 8`, `1024`, `1<<16`.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-12`).
