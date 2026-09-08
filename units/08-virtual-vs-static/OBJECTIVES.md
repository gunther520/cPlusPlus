# Unit 08 — Virtual dispatch vs static

## Target

- Time a **virtual call** in a tight loop over mixed types (the compiler cannot devirtualize).
- Compare **CRTP / templates** (resolved at compile time).
- Put `std::function` on the same work: type erasure + possible heap.
- Know when a vtable is the right trade (plugins, mixed types) and when it is not (hot inner loop).

## Predict

1. Mixed `Base*` virtual calls versus a templated `process(T&)` — which p50 wins at `-O2`?
2. Will `-O0` hide most of the gap? (Re-run `OPT=0`.)
3. Is `std::function` closer to virtual, or to a direct call?

## Run

```bash
cd units
make clean && make run-08 OPT=2
make clean && make run-08 OPT=0
```

Godbolt the `tick` methods with `-O2`. Look for `call *offset(%rax)` (vtable) versus inlined arithmetic.

## What you should see

- At `-O2`, virtual on a mixed array loses to CRTP/template. Direct/inlined work can be vectorized; a call through a pointer usually cannot.
- `std::function` is often the slowest: indirect call *and* a wrapper object (sometimes heap if the callable is large).
- At `-O0`, everything is a call and the ranking may look random. Do not learn dispatch from `-O0`.

If the compiler *can* see a single concrete type, it may devirtualize. That is why this unit uses two derived types in one array.

## Why

- Virtual: load vptr → load function pointer → indirect call. Breaks inlining.
- CRTP: `static_cast<Derived*>(this)->tick()` is a direct call, usually inlined.
- `std::function`: type-erased callable. Fine for callbacks you fire rarely; expensive per tick.

## Wrong conclusions

- “OOP is slow.” A virtual call on a cold path is noise. A virtual call per order in a matching loop is a design choice you must measure.
- Inheriting for *code reuse* and paying a vtable when an `enum` + `switch` or a template would do.
- Storing `std::function` in a hot observer list because it was convenient.

## Exercise

`TODO(unit-08)`: fill the `Base*` array with **only** `TypeA` (comment out the `TypeB` stores).

If Case A speeds up a lot, the compiler (or the CPU’s indirect predictor) is winning because the target is stable. Heterogeneous targets are the expensive case.
