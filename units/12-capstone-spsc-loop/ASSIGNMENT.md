# Unit 12 lab (~1 hour)

After `make run-12`.

## Goal

Move ~200k small messages from one producer thread to one consumer. Replace `mutex + queue` with an **SPSC ring** (copy the idea from `compare.cpp`).

## Run

```bash
cd units
make run-assign-12
taskset -c 0,1 ./bin/assign-12
```

## Tasks

1. Case A: `LockedQueue` (already there). Record p50/p99.
2. Implement `SpscRing<1024>` with padded atomics and acq/rel. Slot type `int` is enough.
3. Same `pump()` as compare.cpp. Ring must not allocate in `try_push`.
4. Try `kCap = 8` vs `4096`. Write what happened.

## Done when

- SPSC p50 beats locked queue (often >5×).
- Checksum/sum of payloads matches (consumer got every int).
- You did not use the ring with two producers.

## Notes

| cap | locked p50 | spsc p50 |
|-----|------------|----------|
| 1024 |  |  |
| 8    |  |  |
