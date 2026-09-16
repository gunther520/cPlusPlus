# Capstone — Mini matching engine

Apply the units on one program: ingest orders, match buy vs sell, measure **p50/p99 of a full replay**.

This is the hands-on assignment. The numbered units taught the pieces. Here you assemble them.

## What you are building

One instrument. Prices are integer ticks. Each order is `{id, side, price, qty}`:

- **Buy** (`side == 0`) lifts asks at `ask.price <= buy.price`, best (lowest) ask first, FIFO at a price.
- **Sell** hits bids at `bid.price >= sell.price`, best (highest) bid first, FIFO at a price.
- Leftover quantity **rests** on the book.

No cancels, no modifications, no two instruments. Correctness first: `filled_qty` and `checksum` must match the naive engine on the shipped workload.

## How to run

```bash
cd capstone
make naive      # baseline (slow on purpose)
make starter    # your code lives in starter/engine.hpp
make reference  # spoilers — optional yardstick after you have a number
make run-naive
make run-starter
```

Each binary prints `fills_qty`, `checksum`, `resting`, and a percentile table for “replay the whole tape.”

Copy the naive line into [RESULTS.md](RESULTS.md) before you change anything.

## Rules (acceptance)

1. **Correctness.** `make run-starter` prints the **same** `fills_qty` and `checksum` as `make run-naive` (same workload seed in `common/workload.hpp`). If they disagree, you changed matching semantics. Stop optimizing.
2. **Faster.** Starter p50 of the replay should be **at least 5×** naive p50 at `-O2`. The reference is a hint at what “layout + no alloc + no mutex” can do — beating it is optional.
3. **Hot path hygiene** (from the units):
   - Measure at `-O2` with warmup (Unit 01, 02).
   - No `new` / `std::string` / `std::list` node per order on the tick (Units 05, 06, 09).
   - No mutex if there is only one matcher thread (Unit 10).
   - No virtual call per order (Unit 08) — templates/`on_order` on a concrete type is enough.
   - No `printf` / iostream inside `on_order` (Unit 13).
   - No `throw` on a miss / partial fill (Unit 20). Prefault the book at startup (Unit 19).
4. **Write-up.** Fill in `RESULTS.md`: naive vs yours, which units you used, one thing that did **not** help.

## Suggested attack order

Do not start with SIMD.

| Step | What to change | Unit |
|------|----------------|------|
| 0 | Run naive, record p50/p99/checksum | 01 |
| 1 | Drop `std::string` side and the mutex | 06, 10 |
| 2 | Kill `std::list` — `vector` of resting orders, or **price-indexed** buckets | 03, 09 |
| 3 | `reserve` / arena so `on_order` does not allocate | 05 |
| 4 | Track best bid/ask; stop scanning the whole book | 07, 09 |
| 5 | (Stretch) generator thread → `SpscOrderRing` → matcher thread; spin, do not `sleep` | 12, 14 |
| 6 | (Stretch) batch or SoA for a *market-data* style scan, not required for matching | 03, 11, 15 |

The starter (`starter/engine.hpp`) **starts as a copy of naive**. Your job is to rewrite `StudentEngine` until it is correct and ≥5× faster. Peek at `reference/` only after you have a number of your own.

## Files

| Path | Role |
|------|------|
| [common/order.hpp](common/order.hpp) | `Order`, `mix_fill` |
| [common/workload.hpp](common/workload.hpp) | Fixed seed tape |
| [common/harness.hpp](common/harness.hpp) | Replay + p50/p99 |
| [common/spsc.hpp](common/spsc.hpp) | Stretch: Unit 12 ring of `Order` |
| [naive/engine.hpp](naive/engine.hpp) | Baseline. Do not “fix” this; compare against it. |
| [starter/engine.hpp](starter/engine.hpp) | **Edit this.** |
| [reference/engine.hpp](reference/engine.hpp) | Spoiler implementation. Peek after you have a number. |

## Stretch goals

- Two threads, SPSC, checksum still matches (Units 04, 10, 12, 14). Pin with `taskset -c 0,1`.
- Shard symbols/instruments across cores or processes (Units 16, 17): one book per shard, mmap or SPSC in, no shared mutex map.
- Runtime ISA / L1 tile for a *market-data* scan (Unit 18): one binary, `target("avx2")` plus `__builtin_cpu_supports`, not `-march=native`. Matching itself is usually pointer chasing, not a float add.
- Latency budget (Unit 21): after a warm book, count dependent hops in one aggressive `on_order` against a 500 ns software envelope.
- Per-order latency: after a warm book, time **one** aggressive order’s `on_order` with Unit 01 percentiles, not only the full replay.
- Prove with `perf stat` that cache-misses dropped vs naive (Unit 03).

Out of scope: DPDK, FPGA, fair lock-free MPMC, multi-rack RPC. If you want those, you already have the vocabulary.

## When you are stuck

Re-read the unit that matches the symptom:

- p50 fine, p99 terrible → measure, alloc, syscalls (01, 05, 13)
- Always slow → layout / list / scan (03, 09)
- Fast but wrong checksum → FIFO / best-price bug, not a compiler issue
- Fast at `-O0` only → you are not measuring a release tick (02)
