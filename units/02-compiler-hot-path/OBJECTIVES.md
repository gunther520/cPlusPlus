# Unit 02 — The compiler is part of the hot path

## Target

- See the same source at `-O0` versus `-O2` (and `-O3`).
- Watch the optimizer **delete work** that is never observed.
- Feel call overhead of a `noinline` function versus an inlined one.
- Know that a bench without `do_not_optimize` is lying.

## Predict

1. At `-O2`, which is faster: Case A (result unused) or Case B (result forced live)? By how much?
2. Will Case A still take real time at `-O0`?
3. Is a 2-argument add cheaper when it cannot be inlined? By a lot, or a little?

## Run

```bash
cd units
make clean
make run-02 OPT=0
make clean
make run-02 OPT=2
make clean
make run-02 OPT=3
```

`make clean` matters: the binary name does not include `OPT`, so a stale `-O2` build would be reused.

Optional: dump the hot loop.

```bash
g++ -std=c++17 -O2 -Icommon -S 02-compiler-hot-path/compare.cpp -o /tmp/u02.s
# search for burn_dead / burn_live / add_noinline
```

Or paste the functions into https://godbolt.org/ (g++, `-O2`).

## What you should see

- **`-O0`:** Case A and Case B both actually execute the loop. Rankings can look odd or flipped; that is why `-O0` is the wrong place to learn performance.
- **`-O2`:** Case A collapses toward empty (often tens of ns — that is *timer overhead*, not 200k adds). Case B still walks the array. That is dead-code elimination, not a faster algorithm.
- The noinline add loop stays visibly more expensive than the inlined one at `-O2`. At `-O0` both are slow because *everything* is a call and there is no register allocation worth mentioning.

## Why

- If a computation cannot affect a later store, syscall, or volatile/asm, ISO C++ lets the compiler delete it.
- Inlining removes call/return, ABI spills, and often enables *further* opts (constant propagation, SIMD).
- `-O0` is for debugging. Publishing `-O0` numbers as “C++ is slow” is a category error.

## Wrong conclusions

- “My new algorithm is 1000x faster” because you forgot to use the result.
- “Virtual calls are free” measured at `-O0` (Unit 08 will show they are not, at `-O2`).
- “Always `-O3` and `-ffast-math`.” `-O3` can help; it can also blow code size and miss I-cache. Measure.

## Exercise

`TODO(unit-02)`: comment out `ll::do_not_optimize(sum)` inside `burn_live` and rebuild at `-O2`.

If Case B now matches Case A, you have proved the harness, not the CPU, was keeping the work alive.
