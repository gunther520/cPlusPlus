# Unit 20 lab (~1 hour)

After `make run-20`.

## Goal

Replace a **throwing** 1% failure path with an **error code** and watch p99 come back.

## Run

```bash
cd units
make clean && make run-assign-20 OPT=2
```

## Tasks

1. Case A: `add_ec`, 1% `b == bad` (already). Record p50/p99.
2. Case B: call `add_ex` in a `try`/`catch` with the same 1% pattern (TODO). Catch `Fail`.
3. Confirm Case B is much slower. That is unwind, not the add.
4. Optional: throw `std::runtime_error` in `add_ex` and compare once. Then put `Fail` back.
5. Optional: move `try` **outside** the `for` loop with 0% fail and compare to per-iteration `try` (happy path).

## Done when

- 1% exceptions lose to error codes on p50 **and** p99.
- You did not throw `std::runtime_error` in a path you would call a tick.
- You wrote whether 0% exceptions matched error codes on *this* ABI (they often do on Linux/g++).

## Notes

| case | p50 | p99 |
|------|-----|-----|
| A error code 1% |  |  |
| B exceptions 1% |  |  |
| 0% both (opt) |  |  |
