# Unit 21 — A latency budget you can measure

## Target

- Build a **back-of-envelope budget** from **this** machine, not a blog’s 2010 table.
- Time a **dependent chase** (`i = next[i]`) in a buffer that fits L1, one that is L2-ish, and one that misses into LLC/DRAM. Sequential scans hide latency (Unit 03 prefetch); a chase does not.
- Translate ns/hop into “how many serialized misses fit in 500 ns of software after the NIC.”

Interview loops use Jeff Dean–style numbers ([order-of-magnitude table](https://quantvault.org/cpp-low-latency-interview-questions.html)). This unit makes you **earn** them.

## Predict

1. 16 KiB chase vs 256 KiB vs 64 MiB: will ns/hop stay ~1 ns, or jump by tens of ns?
2. A 1.5 µs wire-to-wire budget, ~1 µs already spent in kernel-bypass NIC RTT: how many **dependent** DRAM-ish hops remain?
3. Are you allowed more hops if they are **independent** L1 loads (Unit 11/15) instead of a pointer chase (Unit 09)?

## Run

```bash
cd units
make run-21
```

The binary prints ns/hop and `hops that fit in 500 ns`. Copy them into the table below.

## What you should see

- L1-sized cycle: about **1 ns/hop** (a few cycles). Hundreds of hops fit in 500 ns.
- Larger working set: ns/hop climbs (L2, then LLC/DRAM). On a VM the “DRAM” number may still be fat L3 — trust the printout, not the label.
- 500 ns / (64 MiB hop) is a **small integer**. That is why Unit 09 kills lists and why the capstone wants a flat book.

Order-of-magnitude **if you have no measurement yet** (folklore, 4 GHz-ish x86 — **not this VM**):

| Event | Ballpark |
|-------|----------|
| L1 hit | ~1 ns |
| L2 hit | ~4 ns |
| L3 hit | ~10–30 ns |
| DRAM | ~80–100 ns |
| Branch mispredict | ~4–5 ns |
| Uncontended mutex | ~20–40 ns |
| Syscall / kernel `write` | hundreds of ns+ (Unit 13) |
| Kernel NIC | tens of µs |
| Kernel-bypass NIC | ~1–2 µs |

## Why

- Dependent loads cannot start until the previous address is known. Prefetchers guess sequential streams; they do not guess `next[i]`.
- A tick-to-trade budget is subtraction: `T_software = T_slo - T_net`. What remains is **serialized stalls + compute**. Three DRAM misses can spend the whole 500 ns.
- p50 of a sequential add is the wrong currency for “can I chase a pointer in the book?”

## Wrong conclusions

- Using Unit 15’s streaming add as “memory is 0.5 ns/float” in a budget for a pointer-rich matcher. Different access pattern, different stall.
- Mixing a brochure “1 µs NIC” into a budget you measured only in-process ([docs/ARCHITECTURE.md](../../docs/ARCHITECTURE.md)).
- Treating the table above as exact on a hypervisor. Case C here is often L3, not DIMM.
- Spending the budget on DPDK before the software path still does a `std::map` per order (Units 05, 09). Measure the process first.

## Exercise

`TODO(unit-21)`: add a **sequential** scan of the 64 MiB buffer (stride 1) and print ns/element next to the chase.

If sequential is ~0.5–2 ns and the chase is tens of ns, you just measured prefetch vs dependence. The budget for a matcher is the chase column.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-21`).
