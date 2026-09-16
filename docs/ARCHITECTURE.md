# Architecture (one box, one instrument)

This is **venue-side matching**, not a strategy. There is no PnL, no queue-position model, no fees.

```text
  generator / tape
        |  SpscRing<Order>   (Unit 12)
        v
   +-----------+     Risk::allow      +-----------+
   |  matcher  | <------------------> |  limits   |  (precomputed; no map)
   |  1 core   |                      +-----------+
   +-----------+
        |
        |  fills (checksum / log on another core — not in this repo)
        v
     (logger / drop-copy)     write(2) OFF the tick (Unit 13)
```

## Who may do what

| Process / thread | Pin | Allocate | `mmap` | `write` | Block |
|------------------|-----|----------|--------|---------|-------|
| Matcher | isolated core | **startup only** | book, rings | no | spin on empty ring (Unit 14) |
| Generator | sibling core | tape at start | shm ring (Unit 17) | no | spin or stall |
| Logger | other core / node | yes | file | yes | yes |

When the ring is **full**, pick a policy and write it down: **stall** the producer (this course’s `while (!try_push)`), **drop**, or **shed to a huge log**. Silent overwrite is not a policy.

## Sharding

Production shard key is **instrument id**, assigned once at startup, `engine[symbol]`. It is not `unordered_map` in the tick (Unit 16 is the counter-example).

## Restart

This repo does not persist sequence numbers. A real gateway is **idempotent on order id** and can rebuild from a snapshot + seq. Stretch: mmap a seq + checksum every N fills.

## Envelope

Unit 21 measures **dependent hops** on this machine. Do not subtract a brochure “1 µs NIC” unless you timed `sendto` (Unit 13). Software envelope = `ns_per_hop * hops in on_order` after a **warm book**.

## Tapes

| `LL_TAPE` | What it is |
|-----------|------------|
| `uniform` (default) | random add, mixed side — checksum gate |
| `cancels` | ~90% cancel-by-id — closer to a real quote stream |
| `onesided` | all buys — book grows, layout shows up |
