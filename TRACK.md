# Track map

Do the **core** units, then the capstone, then track B if you still want ISA/pages/IPC depth.

## Core (do these, then `capstone/`)

| Unit | Why it is core |
|------|----------------|
| 01 measure | p50/p99, warmup, clocks |
| 02 compiler | `-O2`, DCE, inline |
| 03 cache | layout beats opcodes |
| 04 false sharing | pad shared lines |
| 05 alloc | no `new` on the tick |
| 06 copies/strings | views, SSO, reserve |
| 07 branches | predictor vs `if` |
| 08 virtual vs static | no vtbl per order |
| 09 data-oriented | no pointer-chasing lists |
| 10 atomics | mutex vs atomic; acq/rel |
| 12 SPSC ring | the queue between threads |
| 13 syscalls | no `write` in `on_order` |

```bash
make run-01 run-02 run-03 run-04 run-05 run-06 \
     run-07 run-08 run-09 run-10 run-12 run-13
make -C capstone run-naive
```

## Track B (optional)

| Unit | Topic |
|------|--------|
| 11 | auto-vectorization |
| 14 | spin vs yield vs sleep |
| 15 | SSE2 intrinsics |
| 16 | shard-per-core |
| 17 | socket vs mmap IPC |
| 18 | runtime ISA / L1 tile |
| 19 | pages, TLB, prefault |
| 20 | exceptions vs error codes |
| 21 | measured hop budget |

SIMD (11, 15, 18) is one idea at three altitudes. Pages (19) and the budget (21) are how you stop quoting folklore ns.

## Always

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — boxes and queues, not opcodes
- `make check` — Unit 12 payload sum + capstone naive vs reference on three tapes
- `make tsan` — units 10 and 12 (Unit 17 uses `fork`; TSan+fork is a different tool)
