# Unit 21 lab (~1 hour)

After `make run-21`.

## Goal

Fill a **500 ns software budget** from measured ns/hop on *this* box, then implement the far chase.

## Run

```bash
cd units
make clean && make run-assign-21 OPT=2
```

## Tasks

1. Case A: chase in 16 KiB (already). ns/hop and hops in 500 ns.
2. Case B: same `chase`, but `next` is a **64 MiB** cycle (TODO: `make_cycle((64<<20)/4, seed)`).
3. Copy p50/hop into the table. `hops ≈ 500 / (p50_ns / kSteps)`.
4. Worked question: 1.5 µs SLO, 1 µs NIC. How many Case B hops fit? If your book is a pointer-rich `std::map`, are you already over budget?
5. Optional: sequential scan of 64 MiB (Unit 19/03). Put that ns/elem next to the chase — do not use it as the matcher budget.

## Done when

- Far chase ns/hop is clearly larger than L1 (often 10–50× on this VM).
- You wrote an integer for “hops in 500 ns” for both cases.
- You did not claim DRAM = 100 ns unless Case B actually printed ~100 ns (hypervisors lie).

## Notes

| case | p50 | ns/hop | hops in 500 ns |
|------|-----|--------|----------------|
| A 16 KiB chase |  |  |  |
| B 64 MiB chase |  |  |  |
| sequential 64 MiB (opt) |  |  | n/a |
