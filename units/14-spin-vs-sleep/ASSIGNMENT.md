# Unit 14 lab (~1 hour)

After `make run-14`.

## Goal

Measure **wake policy**: spin vs `yield` vs a short sleep on a ping-pong flag. Know when each is allowed.

## Run

```bash
cd units
make run-assign-14
taskset -c 0,1 ./bin/assign-14
time -p ./bin/assign-14
```

## Tasks

1. Case A: busy spin (already). Record p50.
2. Case B: `yield` in the wait loop (TODO).
3. Case C: `sleep_for(1us)` — use **fewer hops** (already). Do not compare p50 to spin as a speedup if hop counts differ.
4. Optional: `_mm_pause()` in the spin loop. Note user-CPU vs wall time.

## Done when

- Spin wall-clock beats yield (and sleep is a different universe).
- You wrote one line: matching thread vs UI thread — which waits how?
- You did not pin both spinning threads on **one** core and call that a fair test.

## Notes

| wait | hops | p50 wall | CPU note |
|------|------|----------|----------|
| spin |  |  |  |
| yield |  |  |  |
| sleep 1us |  |  |  |
