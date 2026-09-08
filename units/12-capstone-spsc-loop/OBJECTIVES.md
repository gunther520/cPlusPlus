# Unit 12 — Capstone: locked queue vs SPSC ring

## Target

- Move a million integers from one producer thread to one consumer.
- Compare a **`std::mutex` + `std::queue`** with a **single-producer single-consumer ring buffer**.
- Reuse everything you learned: cache lines (pad head/tail), atomics (acquire/release), no heap on the tick, measure p50/p99.
- Know the SPSC contract: **one** pusher, **one** popper. Two producers is a bug, not a faster queue.

## Predict

1. Which design has better p50 to drain `kMessages`?
2. Which has better p99? (Locks + heap growth vs spinning on full/empty.)
3. If the ring is tiny (`kCap = 8`), does the producer stall so much that the lock queue wins? Capacity is part of the design.

## Run

```bash
cd units
make run-12
```

Optional:

```bash
taskset -c 0,1 ./bin/12-capstone-spsc-loop
```

## What you should see

- Locked `std::queue`: mutex + possible heap allocations as the queue grows (Unit 05). Fat tails when both threads contend.
- SPSC ring: preallocated slots, head/tail on separate lines (Unit 04), `release` after write / `acquire` before read (Unit 10). Hot path is a few atomics and a copy into a slot.
- Throughput (messages / wall time) should favor the ring on a quiet two-core pin.

This is not a full matching engine. It is the socket those engines sit on: a wait-free-enough path for *one* writer and *one* reader.

## Why

- SPSC can avoid a lock because there is no third party to exclude. The producer owns `tail`, the consumer owns `head`.
- Publication: write the slot **then** `tail.store(release)` so the consumer’s `tail.load(acquire)` sees the payload.
- Padding `head` and `tail` is Unit 04: they are the hottest shared (actually: independently written) variables in the program.

## Wrong conclusions

- Copying this ring and using it with two producers. You need MPSC/MPMC (much harder) or a mutex.
- Calling it “lock-free” and then allocating inside `try_push`.
- Busy-spinning in a laptop app that should block. Spinning is a latency choice; it burns a core. Production systems often spin a bounded time, then `futex` wait.

## Exercise

`TODO(unit-12)`: drop `kCap` to 8, then raise it to `1<<16`.

- Tiny ring: producer and consumer couple; you measure ping-pong more than queueing.
- Huge ring: extra memory, better decoupling, possibly worse cache if the working set is cold.

Then add a `std::string` payload (or a 64-byte struct) instead of `int`. Copy cost (Unit 06) will start to dominate the queue mechanics. That is the right time to pass indices into a preallocated arena instead of moving big objects through the ring.
