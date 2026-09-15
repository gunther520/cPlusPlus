# Unit 16 lab (~1 hour)

After `make run-16`. This is the **parallel** lab: scale by **not sharing**.

## Goal

Four workers update counters keyed by `id`. Beat a single mutex map with **shards** (`id % 4`, one map per thread, no lock).

## Run

```bash
cd units
make run-assign-16
taskset -c 0-3 ./bin/assign-16
```

## Tasks

1. Case A: 4 threads, one `mutex` + `unordered_map` (already).
2. Case B: 4 independent maps; each thread processes `key % 4 == shard` only.
3. Compare also to **1 thread** on the locked map (often faster than 4 locked threads).
4. Extra: send all keys to shard 0. Sharding should collapse.

## Done when

- Sharded p50 beats 4-thread locked p50.
- You wrote why “more threads” lost on the locked map (serialization + bouncing).
- You did not share one `unordered_map` without a lock (data race).

## Notes

| case | p50 | p99 |
|------|-----|-----|
| 1 thread locked |  |  |
| 4 thread locked |  |  |
| 4 shards |  |  |
| all keys shard 0 (opt) |  |  |
