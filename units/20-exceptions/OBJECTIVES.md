# Unit 20 — Exceptions on the hot path

## Target

- Measure **error codes** versus **C++ exceptions** on the same `noinline` helper.
- See the Itanium “zero-cost” happy path: when nothing throws, `try`/`catch` is often in the noise at `-O2`.
- See the **throw** path: unwind, landing pads, and p99. That is not an error-code return.
- Keep throws off the tick. Rare control flow belongs in a quiet phase, or is an `int` / `expected`-shaped return.

## Predict

1. 200k calls, **0%** failures: error-code helper vs `try`/`catch` per call. Who wins p50?
2. Same loop, **1%** failures (`i % 100 == 0`). Do exceptions still look free?
3. If you throw `std::runtime_error("fail")` instead of a tiny `struct`, what extra cost appears? (Unit 05/06)

## Run

```bash
cd units
make run-20
```

Optional: rebuild the lab with exceptions off and watch Case B fail to compile — that is a valid production mode (`-fno-exceptions`), not a micro-optimization of the happy path.

```bash
# not required; shows the other culture
# g++ -fno-exceptions ...  # throw is a hard error
```

## What you should see

- Case A vs B (0% fail): p50 close. On this course’s g++/Linux, the success path is cheap. Do not quote a blog that measured MSVC `/EHsc` x86 as if it were this ABI.
- Case C vs D (1% fail): exceptions much slower (often ~5–15× here). p99 is the unwind, not the add.
- The helper is `noinline` so the throw is a real call, not `goto` in `main`.

## Why

- Itanium EH (Linux/g++) records unwind tables and leaves the happy path as ordinary code. You pay when you **throw**.
- Unwind is effectively a walk of metadata; historically it also serializes badly across threads (global locks in some libstdc++ paths). A tick that throws under load is a tail-latency incident.
- `std::runtime_error` constructs a `string`. That is Unit 05 inside the exceptional path. A small POD throw is already expensive; a allocating throw is worse.

## Wrong conclusions

- “Exceptions are always slow” from a 0% case that matched error codes. The policy is: **do not throw on the expected path**, not “never catch in `main`.”
- Using `throw` for “not found” in a book lookup 1% of the time. That 1% *is* your p99.
- Catching `...` in the hot function “just in case.” Tables and barriers are still there; you also hid bugs.

## Exercise

`TODO(unit-20)`: throw `std::runtime_error("fail")` instead of `Fail`. Re-run Case D.

If p50 jumps again, you added heap to unwind. Switch back to `Fail` and return an `int` error on the tick.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-20`).
