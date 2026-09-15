# Unit 01 lab (~1 hour)

Do this **after** `make run-01` and reading OBJECTIVES.md.

## Goal

You time **your own** workload (not `burn` from `compare.cpp`) with a warmed percentile bench, and you explain p50 vs p99 in writing.

## Run

```bash
cd units
make run-assign-01
```

## Tasks (about 50 minutes)

1. Open `assignment.cpp`. Case A one-shots a `mix()` loop five times. Case B is a stub.
2. Point Case B at `ll::bench` with warmup ≥ 10 and samples ≥ 100. Keep `ll::do_not_optimize` on the mix result.
3. Change `kWork` by 10× (down, then up). Record p50 and p99/p50 in the table below.
4. Answer: at the **small** `kWork`, is p99/p50 worse? Why? (timer / OS noise vs the loop)

## Done when

- Case B prints min/p50/p99 (not five raw lines only).
- You filled the table for two `kWork` values.
- You can say in one sentence whether you were measuring `mix` or the clock.

## Notes

| kWork | p50 | p99 | p99/p50 | I think I measured… |
|-------|-----|-----|---------|---------------------|
|       |     |     |         |                     |
|       |     |     |         |                     |
