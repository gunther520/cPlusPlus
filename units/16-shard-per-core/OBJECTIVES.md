# Unit 16 — Shard-per-core (parallel without sharing)

## Target

- See that **threads + one mutex** is often slower than **one thread**, not faster, when the critical section is tiny.
- Compare a shared `unordered_map` + lock with **sharded maps** (key % N, no sharing).
- Learn the low-latency parallel rule: **do not share mutable cache lines**. Give each core its own data (pipeline or shard). That is not MapReduce; it is how matching engines scale.

## Predict

1. Four threads updating one mutex-protected map vs four threads each owning a shard: who wins p50?
2. Will four threads on one map beat a **single** thread on that map? (Contention can make “more cores” lose.)
3. If keys are skewed onto shard 0, does sharding still help?

## Run

```bash
cd units
make run-16
taskset -c 0-3 ./bin/16-shard-per-core
```

## What you should see

- Shared mutex map: p50 and especially p99 suffer (lock + Unit 04 true sharing of the map).
- One thread, no lock: often beats the 4-thread locked version.
- Four shards, no lock: best throughput here if keys hash evenly.

This is parallel processing for latency: **isolation**, not a bigger lock.

## Why

- A mutex serializes the hot section. Four cores then take turns plus cache bounce.
- Sharding makes the problem embarrassingly parallel: no shared line except the input tape (read-only).
- Distributed *in one box* looks the same: one matcher process per shard, SPSC in (Unit 12), no shared book.

## Wrong conclusions

- “Always more threads.” On a contended map you paid for cores that wait.
- `std::async` over a shared object as a scalability plan.
- Sharding without a routing rule (sticky key → sticky core). Random steal across shards brings the mutex back.

## Exercise

`TODO(unit-16)`: force every key onto shard 0 (`key % 1`).

The four “shard” threads collapse onto one map plus wasted workers. p50 should look like the single-thread case (or worse). Then restore `% kShards`.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-16`).
