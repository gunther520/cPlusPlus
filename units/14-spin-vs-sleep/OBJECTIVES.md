# Unit 14 — Spin vs sleep (waiting for a message)

## Target

- Measure **wake latency** when a producer publishes a flag the consumer waits on.
- Compare a **busy spin**, `std::this_thread::yield()`, and `sleep_for`.
- Connect to Unit 12: the SPSC ring spins on empty/full. That is a latency choice that **burns a core**.

## Predict

1. Pure spin vs `yield` vs `sleep_for(1us)`: who has the best p50 from publish to observe?
2. Who has the worst p99? (The scheduler is allowed to park you.)
3. Should a matching thread sleep in production? Should a laptop UI thread spin?

## Run

```bash
cd units
make run-14
taskset -c 0,1 ./bin/14-spin-vs-sleep
```

## What you should see

- **Spin:** best p50/p99 (tens of ns to a few hundred ns on a quiet box). CPU ~2 cores pegged.
- **yield:** slower and noisier; you donate the core, the OS may run someone else.
- **sleep_for:** p50 often **> 50 µs** even if you asked for 1 µs. Timer slack and the scheduler dominate. Useless for HFT ticks; fine for batch jobs.

Absolute numbers depend on the kernel tick and whether the two threads run on different cores.

## Why

- Spin: the consumer’s load sits in a loop; the producer’s `store(release)` is visible on the next iteration (Unit 10). No syscall.
- `yield` / `sleep`: `sched_yield` / `nanosleep` → kernel. Wakeup is a **scheduling** event, not a cache-coherence event.
- Unit 12’s ring uses spin because the assignment is latency. Production often **spins a bounded number of times, then futex-waits** so an idle process does not melt a rack.

## Wrong conclusions

- Spinning always. On a machine that shares cores with the OS and other apps, you steal cache and power for a 1 Hz task.
- `sleep_for(0)` as a portable “pause.” It is still a syscall-shaped hint, not a pause instruction.
- Pinning both threads on **one** core and then spinning: they cannot run at the same time, p99 explodes.

## Exercise

`TODO(unit-14)`: in the spin waiter, insert `_mm_pause()` (x86) inside the loop.

On SMT/HT this usually **improves** neighbor-thread performance without much hurting p50. Then raise `kHops` and compare CPU time (`time -p ./bin/14-spin-vs-sleep`). Spin wall-clock stays low; user-CPU stays high. Sleep is the opposite.
