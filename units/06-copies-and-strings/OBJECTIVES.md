# Unit 06 — Copies, SSO, and strings

## Target

- See extra copies of large objects dominate a loop that “only looks at data.”
- Use `std::string_view` (or `const std::string&`) on the read path.
- Watch **small string optimization (SSO)**: tiny strings stay off the heap; slightly larger ones do not.
- `reserve` a string you will append to, same lesson as Unit 05.

## Predict

1. `sink(std::string)` by value versus `sink(std::string_view)` on a 256-byte payload: who wins?
2. Appending one character `kN` times without `reserve` versus with `reserve(kN)`.
3. Is a 3-character string heap-allocated on libstdc++? Is a 32-character string?

## Run

```bash
cd units
make run-06
```

## What you should see

- By-value `std::string` copies (and for long strings: heap alloc + memcpy) every call.
- `string_view` is pointer + length. The timed path should not allocate.
- Unreserved append: geometric growth (Unit 05 again), plus SSO → heap transition when the string outgrows the inline buffer.
- Short-string Case E should be close to “no heap”; long-string Case F pays `new`.

SSO buffer size is implementation-defined (often 15–23 bytes on libstdc++ 64-bit). The program prints `sizeof(std::string)` to hint at layout.

## Why

- `std::string` owns memory. Passing it by value copies the object; if the payload is not inside SSO, it also allocates.
- `string_view` does not own. It is only valid while the underlying buffer lives — fine for a parse of a buffer you already hold.
- Append without `reserve` is the vector story: allocate, copy, free, repeat.

## Wrong conclusions

- “Always `string_view`.” If you need to keep the text after the source dies, you must own a `string` (or intern it).
- Returning `string_view` into a function-local `string`. Dangling view, not a speedup.
- Microbenching SSO strings and claiming “C++ strings are free.” Cross the SSO threshold.

## Exercise

`TODO(unit-06)`: bump `kLong` from 64 toward 8, then toward 4096.

- Where does Case A suddenly get much worse? That is the SSO → heap edge plus copy cost.
- At `kLong == 8`, by-value vs view should be closer: you are copying a few bytes in registers / the string object, not a heap buffer.
