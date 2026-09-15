# Unit 05 lab (~1 hour)

After `make run-05`.

## Goal

Remove heap traffic from a hot builder: `reserve` a vector, then replace `new` nodes with a **bump pool**.

## Run

```bash
cd units
make run-assign-05
```

## Tasks

1. Case A: `push_back` `kN` ints with no `reserve`, and a `new` chain of nodes.
2. Add `reserve(kN)` for the vector case.
3. Implement `Pool::alloc` using a pre-sized `vector<Node>` (see compare.cpp). Reset each sample.
4. Record speedups. Optional: uncomment `shrink_to_fit` after clear and watch p99 get worse.

## Done when

- Reserved vector beats unreserved p50.
- Pool beats new/delete p50 by a large factor (often >10×).
- You did not allocate inside the timed `alloc` after `Pool` construction.

## Notes

| case | p50 | p99 |
|------|-----|-----|
| vector no reserve |  |  |
| vector reserve    |  |  |
| new/delete        |  |  |
| pool              |  |  |
