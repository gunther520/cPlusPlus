# Unit 10 lab (~1 hour)

After `make run-10`.

## Goal

1. Beat a contended mutex counter with `atomic fetch_add`.
2. Publish a payload with **release/acquire**, not relaxed.

## Run

```bash
cd units
make run-assign-10
```

Optional: `g++ -fsanitize=thread` on your publish pair after you “fix” it (and after you break it with relaxed).

## Tasks

1. Case A: two threads, mutex `++`. Case B: `fetch_add` relaxed. Record p50.
2. Case C: producer writes `payload` then a flag; consumer spins on the flag then reads `payload`. Start with **relaxed** on the flag (TODO). Switch to release/acquire.
3. Write why relaxed is legal for the counter and illegal for the payload.

## Done when

- Atomic counter beats mutex p50 under two threads.
- Publish uses `release`/`acquire` (or seq_cst). You did not “optimize” that away.
- You did **not** add a data race on a plain `int` as a “fast path.”

## Notes

| case | p50 | p99 |
|------|-----|-----|
| mutex 2T |  |  |
| atomic relaxed 2T |  |  |
| payload+flag |  |  |
