# Unit 08 lab (~1 hour)

After `make run-08 OPT=2` (and once at `OPT=0` so you do not trust it).

## Goal

Replace a mixed virtual call loop with **CRTP or an enum + switch** (or two homogeneous loops). `std::function` is the trap.

## Run

```bash
cd units
make clean && make run-assign-08 OPT=2
```

## Tasks

1. Case A: `Base*` array, mixed `TypeA`/`TypeB`, virtual `tick`.
2. Case B: implement CRTP (or `enum class Kind` + switch) on the same deltas.
3. Optional: `std::function` with a **fat capture** — usually worse than virtual.
4. Confirm `-O0` ranking is noise.

## Done when

- At `-O2`, static/CRTP p50 beats mixed virtual.
- Checksums / final `s` match (same +1/+2 pattern).
- You did not “win” by letting the compiler delete the loop (`do_not_optimize`).

## Notes

| OPT | virtual p50 | CRTP/switch p50 |
|-----|-------------|-----------------|
| 0   |             |                 |
| 2   |             |                 |
