# Capstone — Mini matching engine

This is **venue-side matching** (one book), not a trading strategy. There is no PnL.
See [docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md). Do the [core track](../TRACK.md) first.

Apply the units: ingest orders (and **cancels**), match buy vs sell, measure **whole-tape** and **per-order after a warm book**.

## What you are building

One instrument. Prices are integer ticks in `[kMinPx, kMaxPx]` (`common/order.hpp`). Each order is `{id, side, price, qty, action}`:

- **Add** (`action == 0`): buy lifts asks at `ask.price <= buy.price` (best ask first, FIFO at a price). Sell hits bids symmetrically. Leftover **rests**.
- **Cancel** (`action == 1`): remove the resting order with this `id` if it is still on the book (no-op if already filled).

No modifications, no two instruments. Correctness first: `filled_qty`, `checksum` (**includes price**), and `resting` must match naive on the tape.

## How to run

```bash
cd capstone
make naive && make run-naive
make starter && make run-starter
make reference && make run-reference   # spoilers
make check                             # naive == reference on uniform, cancels, onesided
LL_TAPE=cancels make run-naive
LL_TAPE=onesided make run-starter
```

Each binary prints checksum plus **two** timing lines:

- `replay whole tape` — includes constructing the engine (setup).
- `per-order after 2/3 warm` — **the tick metric**: one `on_order` at a time on a live book.

Copy naive into [RESULTS.md](RESULTS.md) before you change starter.

## Rules (acceptance)

1. **Correctness.** Same `fills_qty`, `checksum`, and `resting` as naive on `LL_TAPE=uniform` **and** `LL_TAPE=cancels` (`make check` covers naive vs reference; you compare starter by hand or run both).
2. **Faster.** Starter whole-tape p50 at least **5×** naive at `-O2`. The number that matters for a desk is the **per-order** line.
3. **Hot path hygiene:**
   - No `new` / `std::string` / `std::list` node per add (Units 05, 06, 09).
   - No mutex if there is only one matcher thread (Unit 10).
   - No virtual call / `throw` / `printf` in `on_order` (Units 08, 13, 20).
   - Prefault / `reserve` the book at startup (Units 05, 19).
   - Call `Risk::allow` (already in naive); do not replace it with a map lookup.
4. **Write-up.** `RESULTS.md`: naive vs yours, tapes you ran, which units helped, one thing that did **not**.

## Suggested attack order

Do not start with SIMD.

| Step | What to change | Unit |
|------|----------------|------|
| 0 | Run naive on `uniform` and `cancels`; record both timing lines | 01 |
| 1 | Drop `std::string` and the mutex | 06, 10 |
| 2 | Kill `std::list` — price-indexed vectors | 03, 09 |
| 3 | `reserve` / arena; handle cancel without a tick `new` | 05 |
| 4 | Track best bid/ask | 07, 09 |
| 5 | Stretch: generator thread → `SpscOrderRing` → matcher; spin | 12, 14 |
| 6 | Stretch: `LL_TAPE=onesided` should show layout more than SIMD | 03, 21 |

The starter **starts as a copy of naive**. Peek at `reference/` only after you have a number. The spoiler’s level compact (`erase` from the front) is a **cheat**; a free-list of slots would not copy.

## Files

| Path | Role |
|------|------|
| [common/order.hpp](common/order.hpp) | `Order`, `kMinPx`/`kMaxPx`, `mix_fill` (id, rest, **price**, qty) |
| [common/risk.hpp](common/risk.hpp) | `Risk::allow` — precomputed, no map |
| [common/workload.hpp](common/workload.hpp) | `LL_TAPE=uniform\|cancels\|onesided` |
| [common/harness.hpp](common/harness.hpp) | Replay + per-order p50/p99 |
| [common/spsc.hpp](common/spsc.hpp) | `SpscOrderRing` = Unit 12 ring of `Order` |
| [naive/engine.hpp](naive/engine.hpp) | Baseline |
| [starter/engine.hpp](starter/engine.hpp) | **Edit this.** |
| [reference/engine.hpp](reference/engine.hpp) | Spoiler |

## Stretch

- Two threads, SPSC, checksum still matches. `taskset -c 0,1`.
- `engine[symbol]` array (not a hot `unordered_map`).
- Persist seq + checksum every N fills (restart).
- `perf stat` cache-misses vs naive.

Out of scope: DPDK, FPGA, MPMC, multi-rack RPC.

## When you are stuck

- p50 fine, p99 terrible → alloc, syscalls, faults (01, 05, 13, 19)
- Always slow → list / full-book scan (03, 09)
- Fast but wrong checksum → FIFO, cancel-of-wrong-id, or dropped price (do not drop: `Risk::allow` is the contract)
- Fast at `-O0` only → not a release tick (02)
