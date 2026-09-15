# Unit 13 — Syscalls on the hot path

## Target

- See that **I/O in the tick** (even `std::ostream`) is a kernel call plus formatting, not “just printing.”
- Compare per-event unbuffered `fprintf` vs an in-memory buffer vs **no log at all**.
- Treat logging as a **sample**, not a requirement of the hot path (Unit 05: allocate/format before or after the event).

## Predict

1. Unbuffered `fprintf` to `/dev/null` for 20k events vs `buf.append(...)` with `reserve` and no `write`: who owns p50? p99?
2. Will p99 of Case A look like a few slow samples (scheduler inside `write`) rather than every call being equally slow?
3. Is “I need logs” a reason to format text inside `on_order`? What would Unit 12 do instead?

## Run

```bash
cd units
make run-13
```

Optional:

```bash
strace -c ./bin/13-syscalls-hot-path
```

You should see many `write` syscalls in Case A (`strace -c` on the binary) and almost none in B/C (beyond startup). Case A writes to `/dev/null` so the terminal is not the bottleneck — the **syscall** is.

## What you should see

- Case A (unbuffered `fprintf` to `/dev/null`): much slower, often uglier p99 (`write` per event).
- Case B (append to a `string` with `reserve`): microseconds, almost no kernel.
- Case C (only a running checksum): faster still — that is the actual matching work with **zero** observability tax.

Redirecting stdout (`./bin/13-syscalls-hot-path > /tmp/out`) can change Case A a lot. That is the lesson: the kernel and the tty are in your critical path.

## Why

- `operator<<` formats, may allocate (Unit 06), then `write(1, ...)`. Crossing into the kernel can sleep.
- A reserved buffer is just stores. You can hand that buffer to a logger thread later (Unit 12 ring of log lines).
- Low-latency shops: **binary** logs, or sampled counters, never text I/O on the match/wire thread.

## Wrong conclusions

- “Always silence the process.” You still need ops. Move I/O **off** the tick.
- Benchmarking with stdout on a slow terminal and declaring C++ I/O “impossible.”
- `std::ios::sync_with_stdio(false)` as the whole fix — it helps iostream, it does not remove the `write`.

## Exercise

`TODO(unit-13)`: in Case B, remove `s.reserve(...)`.

p99 should jump (growth + copies, Unit 05). Then restore `reserve`, and try `s.push_back` of a binary `uint32_t` instead of decimal text. Formatting is often as expensive as the syscall.
