# Unit 09 — Data-oriented layout (pointer chasing)

## Target

- Sum the same integers from a **linked list** versus a **`std::vector` of values**.
- Feel pointer chasing: each node can sit on a different cache line, and the next address is data-dependent (no prefetch).
- Reuse Unit 03: locality is a layout problem, not a “C++ is slow” problem.
- Prefer **indexes into an array** over graphs of heap nodes on the hot path.

## Predict

1. List vs vector, same N, same values: who wins, and is it close?
2. If you allocate list nodes back-to-back (an arena) but still follow `next` pointers, does that close most of the gap?
3. Will p99 of the list look especially bad? (TLB + misses)

## Run

```bash
cd units
make run-09
```

Optional:

```bash
perf stat -e cache-misses,dTLB-load-misses ./bin/09-data-oriented
```

## What you should see

- Vector of `int`: sequential scans, hardware prefetch, full cache lines of payload.
- Heap-allocated list: one payload `int` per node plus pointers; nodes scattered; next line unknown until the load completes. Often an order of magnitude slower at large N.
- Arena list (Case C): better than scattered `new`, still worse than the vector — you still serialize on `next` and waste line capacity on pointers.

## Why

- A load that depends on the previous load’s result cannot start early. That is the pointer-chasing ceiling.
- `vector<int>` is also “a data structure,” just one the CPU likes.
- Many “tree/graph in the matcher” designs accidentally become this unit in production.

Your earlier AVL tree in this repo was a great correctness exercise and a classic pointer-chasing container. Low-latency order books usually keep levels in arrays / maps with stable nodes, not a fresh pointer walk per tick.

## Wrong conclusions

- “Linked lists are O(n) so they must be as fast as vector scans.” Big-O hides the constant, and the constant here is DRAM.
- `std::list` “because I insert in the middle” without asking how often that happens on the timed path.
- Allocating nodes from a pool and declaring the design data-oriented — the *access pattern* still chases pointers.

## Exercise

`TODO(unit-09)`: shrink `kN` to 64 (fits in L1) and re-run.

The list should get closer to the vector: misses disappear. Then set `kN` back large. If the gap only exists at large N, you have isolated memory, not “instruction overhead of `next`.”
