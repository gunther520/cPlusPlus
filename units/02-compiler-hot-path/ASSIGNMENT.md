# Unit 02 lab (~1 hour)

After `make run-02 OPT=0` and `make run-02 OPT=2`.

## Goal

Prove that `-O2` can delete work, then keep the work with `do_not_optimize`. Compare inlining on a tiny function.

## Run

```bash
cd units
make clean && make run-assign-02 OPT=0
make clean && make run-assign-02 OPT=2
```

## Tasks

1. Case A calls `work()` and ignores the return. Case B is supposed to keep the result live — it does not yet.
2. Add `ll::do_not_optimize` (or use the return) in Case B. At `-O2`, Case A should collapse; Case B should not.
3. Time `add_noinline` vs `add_inline` loops already in the file. Record the `-O2` ratio.
4. Optional: `g++ -O2 -S assignment.cpp` and find whether `work` disappeared.

## Done when

- At `-O2`, unused `work` is ~ns (DCE) and kept-live `work` is clearly slower.
- At `-O0`, both `work` cases actually run.
- You wrote the inline vs noinline p50 ratio below.

## Notes

| OPT | unused p50 | kept p50 | noinline p50 | inline p50 |
|-----|------------|----------|--------------|------------|
| 0   |            |          |              |            |
| 2   |            |          |              |            |
