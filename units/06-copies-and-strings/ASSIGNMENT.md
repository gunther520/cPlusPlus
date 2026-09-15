# Unit 06 lab (~1 hour)

After `make run-06`.

## Goal

Parse a fake `k=v;` tape without allocating a `std::string` per field. Use `string_view` (or pointer+length). `reserve` a sink buffer if you must copy.

## Run

```bash
cd units
make run-assign-06
```

## Tasks

1. Case A: for each token, `std::string field = token;` then hash it.
2. Case B: hash `string_view` into the original buffer. No per-token heap.
3. Optional: build one output string with `reserve` vs repeated `+=` of 32-byte chunks.

## Done when

- View/hash path p50 is clearly faster than by-value `string` (often >5× for long tokens).
- You wrote whether SSO hid the bug for 3-character tokens (try `kTok=3` vs `64`).

## Notes

| kTok | by-value p50 | view p50 |
|------|--------------|----------|
| 3    |              |          |
| 64   |              |          |
