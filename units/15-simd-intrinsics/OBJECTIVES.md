# Unit 15 — SIMD by hand (intrinsics)

## Target

- Write an explicit **SSE2** add (`_mm_add_ps`) and compare it to a scalar loop the compiler is **not** allowed to vectorize (`noinline` + opaque).
- See that Unit 11’s auto-vectorizer and this unit’s intrinsics are the same hardware; intrinsics are for when the compiler will not go there.
- Know the portable fallback: scalar tail when `n` is not a multiple of the lane count.

## Predict

1. At `-O2`, will the intrinsic loop beat a `noinline` scalar add of 1M floats?
2. At `-O3 -march=native`, does auto-vec close the gap so much that hand SSE is a wash?
3. Why do we `loadu` / `storeu` instead of aligned `load_ps`?

## Run

```bash
cd units
make clean && make run-15 OPT=2
make clean && make run-15 OPT=3
```

Dump asm:

```bash
g++ -std=c++17 -O2 -Icommon -S 15-simd-intrinsics/compare.cpp -o /tmp/u15.s
# look for addps / xmm
```

## What you should see

- Case A (scalar, noinline): one add per float.
- Case B (SSE2, 4 floats per `addps`): often ~2–4× at `-O2` on this box.
- Case C (plain loop in the same file): gcc may already emit SIMD at `-O2`/`-O3`. This unit uses a **smaller** `kN` so you are not purely DRAM-bound; if you raise `kN` to `1<<20` the three cases often meet (memory).

x86_64 always has SSE2. AVX (`ymm`) is optional; this unit stays on 128-bit so it runs everywhere this course runs.

## Why

- A 128-bit XMM register holds 4 × 32-bit floats. `_mm_add_ps` is four adds.
- `loadu` allows any address. Aligned loads fault if you lie about alignment.
- Intrinsics are **not** faster than a compiler that already vectorized the same loop. They are a tool when aliasing, a branch, or a call blocked auto-vec (Unit 11).

## Wrong conclusions

- Hand-written SIMD as the first optimization. Layout (Unit 03) and allocations (Unit 05) move p99 more often.
- Mixing AVX and SSE without `vzeroupper` in a large binary (context-switch tax). Stay consistent.
- Assuming ARM has `_mm_add_ps`. This file is x86-only; the scalar cases still run elsewhere.

## Exercise

`TODO(unit-15)`: unroll Case B to 8 floats per iteration (two `__m128` adds).

If p50 drops, you were instruction-bound. If not, you were memory-bound (1M floats × 3 arrays ≈ 12 MiB — last-level cache / DRAM). That tells you whether to write more SIMD or to **touch less data**.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-15`).
