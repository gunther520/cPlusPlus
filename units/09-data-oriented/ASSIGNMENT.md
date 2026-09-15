# Unit 09 lab (~1 hour)

After `make run-09`.

## Goal

Sum the same integers from a **heap linked list**, an **arena list**, and a **`vector<int>`**. Isolate pointer chasing from allocation.

## Run

```bash
cd units
make run-assign-09
```

## Tasks

1. Case A: `new` per node, sum via `next`.
2. Case B: nodes in one `vector<Node>` still following `next` (arena).
3. Case C: `vector<int>` sum (already sketched).
4. Repeat with `kN = 64`. The gap should shrink (cache).

## Done when

- Vector beats heap list at large `kN` (several ×).
- Arena list is between them or near heap — still chasing `next`.
- At `kN=64` you recorded that the gap collapsed.

## Notes

| kN | heap list p50 | arena p50 | vector p50 |
|----|---------------|-----------|------------|
| 1<<20 |  |  |  |
| 64    |  |  |  |
