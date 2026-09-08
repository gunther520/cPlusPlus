# Unit 11 — Vectorization (let the compiler use SIMD)

## Target

- See a loop the compiler **cannot** auto-vectorize (aliasing or a data-dependent branch).
- See the same arithmetic in a **contiguous, side-effect-free** loop it can vectorize.
- Know `__restrict__` as a promise: “these pointers do not overlap.”
- Inspect asm (`-O3 -march=native`) for `addps` / `vfmadd` versus scalar `addss`.

## Predict

1. `c[i] = a[i]+b[i]` with possible alias versus `__restrict__` pointers — who wins at `-O3`?
2. A loop with `if (mask[i]) acc += a[i]` versus a branchless `acc += a[i] * mask[i]` — this is the comparison that usually moves a lot.
3. Will `-O0` show any SIMD gap? (It should not.)

## Run

```bash
cd units
make clean && make run-11 OPT=3
make clean && make run-11 OPT=0
```

Dump asm (look for xmm/ymm/zmm):

```bash
g++ -std=c++17 -O3 -march=native -Icommon -S \
  11-vectorization/compare.cpp -o /tmp/u11.s
```

Or Godbolt with `-O3 -march=native`.

## What you should see

- At `-O3`, the **branchless** masked sum should beat the branchy one (often several times). GCC may already vectorize a plain `c[i]=a[i]+b[i]` *with a runtime overlap check*, so Case A vs B can look close — that is loop versioning, not “restrict is useless.” Read the asm.
- At `-O0`, all cases look similar and terrible. Vectorization is an optimizer feature.

Gaps vary by CPU (`-march=native` matters). If everything is already vectorized, read the asm; the lesson still holds.

## Why

- SIMD applies the same operation to 4–16 floats per instruction. That needs contiguous data, independent iterations, and no overlapping stores.
- A data-dependent branch is “different work per lane.”
- Aliasing: if `c` might equal `a+1`, a vector store could be illegal. Without `restrict`, GCC must be conservative.

This unit stops at **auto-vectorization**. Hand-written intrinsics and ISA-specific kernels are a later course.

## Wrong conclusions

- `#pragma omp simd` as a substitute for layout. If data is AoS with a branch, pragmas will not save you (Units 03, 07).
- `restrict` on pointers that *do* overlap — that is undefined behavior, not a speedup.
- Enabling `-ffast-math` to “get SIMD” without checking whether associativity of your reductions is acceptable.

## Exercise

`TODO(unit-11)`: in Case A, pass `add_maybe_alias(a, a, c, n)` so `b == a` (still legal) versus `add_maybe_alias(c+1, a, c, n)` overlap (illegal for `restrict`).

Keep Case B honest: only use `restrict` when you can prove no overlap. Then re-run Case C/D after making `mask` all ones — the branch becomes predictable (Unit 07) and may suddenly vectorize or `cmov`.
