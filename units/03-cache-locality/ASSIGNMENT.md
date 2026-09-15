# Unit 03 lab (~1 hour)

After `make run-03`.

## Goal

Convert a tiny **AoS** particle update to **SoA** and show a p50 win when you only touch `x` (and maybe `y`), not the whole particle.

## Run

```bash
cd units
make run-assign-03
```

## Tasks

1. Case A updates `p[i].x` on `vector<Particle>` (AoS). Case B is a copy of that.
2. Build SoA (`vector<float> x,y,mass,pad`) and update only `x[i] += vx`.
3. Record AoS vs SoA p50. Optional: `perf stat -e cache-misses` both ways.
4. Extra: also write `y` every iteration. Does the SoA win shrink? (use case)

## Done when

- SoA p50 is faster for the **x-only** loop at the shipped `kN`.
- You wrote one sentence: when you would keep AoS instead.

## Notes

| layout | p50 | p99 | cache-misses (optional) |
|--------|-----|-----|-------------------------|
| AoS    |     |     |                         |
| SoA    |     |     |                         |
