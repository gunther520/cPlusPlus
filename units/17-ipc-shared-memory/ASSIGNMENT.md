# Unit 17 lab (~1 hour)

After `make run-17`. This is the **distributed** lab: two **processes** on one machine (gateway vs matcher), not a cloud cluster.

## Goal

Ping-pong an 8-byte token between parent and child. Beat `socketpair` with a **shared-memory SPSC** (`mmap MAP_SHARED`).

## Run

```bash
cd units
make run-assign-17
```

## Tasks

1. Case A: `socketpair` + `fork` + `send`/`recv` (already).
2. Case B: two rings in `mmap` (forward and reverse), spin wait, same hop count. You may copy helpers from `compare.cpp`.
3. Extra: 64-byte payload. Socket usually loses more.
4. Write why a 200 µs network RTT would dominate every optimization in Units 01–16.

## Done when

- Shm ring p50 beats socket (often several ×) with spin waits.
- You `waitpid` the child; no zombie, no data race on a non-atomic flag.
- You stated: WAN/RPC is a different latency class; colocate first.

## Notes

| path | bytes | p50 | p99 |
|------|-------|-----|-----|
| socket | 8 |  |  |
| shm ring | 8 |  |  |
| socket | 64 (opt) |  |  |
| shm ring | 64 (opt) |  |  |
