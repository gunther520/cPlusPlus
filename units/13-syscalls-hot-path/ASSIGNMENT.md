# Unit 13 lab (~1 hour)

After `make run-13`.

## Goal

Take I/O off the tick: replace per-event `fprintf` with a **reserved memory log**. Do not spam a real tty.

## Run

```bash
cd units
make run-assign-13
```

## Tasks

1. Case A: unbuffered `fprintf` to `/dev/null` per event (already there).
2. Case B: `string::append` + `to_string` **with** `reserve`.
3. Extra: append binary `uint32_t` (4 bytes) instead of decimal text. How much was formatting?
4. Optional: `strace -c ./bin/assign-13` — Case A should show many `write`s.

## Done when

- In-memory log p50 is several × faster than fprintf.
- You did not “fix” Case A by buffering into huge stdio buffers without saying so.
- Notes include: logs belong on another thread / after the event (Unit 12 ring of lines).

## Notes

| case | p50 | p99 |
|------|-----|-----|
| fprintf unbuffered |  |  |
| reserved text log  |  |  |
| binary append (opt)|  |  |
