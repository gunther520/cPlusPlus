# Unit 18 — Hardware-adaptive optimization

## Target

- Ship **one binary** that picks a kernel at **startup** from the CPU in front of it (`__builtin_cpu_supports`), instead of compiling with `-march=native` and SIGILL-ing on the next box.
- Keep wide SIMD (`AVX2` / `ymm`) inside a function marked `__attribute__((target("avx2")))`. The rest of the file stays at the x86-64 baseline (SSE2) so the binary still loads on older machines.
- Size a **blocked column walk** from the **L1 data cache** in sysfs, not from a constant that matched the author’s laptop.

This is what production math libraries, codecs, and HFT feed handlers actually do: detect once, then call through a function pointer for the rest of the process.

## Predict

1. At `-O2` **without** `-mavx2` on the command line, can Case B (dispatched) still beat a `noinline` scalar Horner polynomial? Who is the picked kernel on *this* box?
2. Forced SSE2 vs dispatched AVX2: with a few mul-adds per float (not a single add), is 256-bit a win at `kN=1<<15`?
3. 1024×1024 **column-major** sum vs the same sum with a strip width derived from L1d. Who owns p50? Why is a tile of `1` (pure column walk) the cache disaster, and why is `3` only a partial fix?

## Run

```bash
cd units
make clean && make run-18 OPT=2
```

Do **not** add `-mavx2` / `-march=native` to `CXXFLAGS` for this unit. The point is that AVX2 lives only in the `target` function.

Dump asm and confirm the split:

```bash
g++ -std=c++17 -O2 -Icommon -S 18-hw-adaptive/compare.cpp -o /tmp/u18.s
# poly_avx2 should contain vmulps/vaddps ymm; poly_scalar / main should not
```

CPU and cache the OS reports:

```bash
grep -m1 flags /proc/cpuinfo
cat /sys/devices/system/cpu/cpu0/cache/index0/{type,level,size}
```

## What you should see

- Startup print: `sse2` / `avx2` (and L1d in KiB). On this course’s x86_64 VM, AVX2 is typically present; the dispatcher should pick `poly_avx2`.
- Case A (scalar, `noinline` + `no-tree-vectorize`): one Horner chain per float.
- Case B (runtime pick): often ~2–4× vs scalar. A single `add_ps` at `1<<15` is often DRAM-bound (Unit 15); the extra Horner steps are there so AVX2 vs SSE2 can still move.
- Case C (always SSE2): slower than AVX2 when 256-bit is legal; **the same as B** if the CPU has no AVX2.
- Case D vs E: column-major 1024×1024 is Unit 03’s stride (`kCols` floats). An L1-derived strip (consecutive columns, then the next row) should be several times faster. Hypervisor cache numbers can be odd — trust the printout in **this** process.

## Why

- `-march=native` bakes **this build machine’s** ISA into every function. A coworker’s older CPU, a container baseline, or a different cloud generation then faults on an illegal opcode.
- `__builtin_cpu_init()` + `__builtin_cpu_supports("avx2")` reads CPUID. Call it **once** at startup; do not CPUID in the tick.
- `__attribute__((target("avx2")))` is how GCC compiles **one** function with extra ISAs while the translation unit stays portable. Taking the address of that function (the dispatcher) keeps it from being inlined into a non-AVX caller.
- After AVX, `_mm256_zeroupper()` avoids the SSE/AVX transition penalty Unit 15 warned about.
- `/sys/devices/system/cpu/cpu0/cache/index*/` is the kernel’s view of L1d/L2/L3. Tile width ≈ “how many consecutive columns keep `rows * tile * 4` bytes in L1.” A constant `16` is a guess about someone else’s L1. Do not use L3 (shared, huge) as the strip.

ARM is the same idea with different names (`sve`, `neon`, cache geometry via sysfs). This file is x86-only for the SIMD kernels; scalar + sysfs still run elsewhere.

## Wrong conclusions

- Dispatch as the **first** optimization. Layout (Unit 03) and allocations (Unit 05) still move p99 more often. This unit is how you **use** SIMD/tiling on a fleet, not a substitute for measuring.
- Calling AVX intrinsics from a function that is **not** `target("avx2")` while compiling without `-mavx2`: either a compile error or illegal instructions in the default ISA path.
- Detecting every call (`if (avx2) poly_avx2(); else poly_sse2();` inside the timed loop). That is a branch plus CPUID-shaped thinking. Pick a pointer once.
- Copying L3 as the tile. L3 is shared and huge; the reuse this demo wants is **L1d per core**.
- “Adaptive” meaning a feedback controller in the hot path. Startup detection + a table of kernels is the usual low-latency version.
- Expecting AVX2 to beat SSE2 on a pure streaming add of megabytes. Memory hides the ISA. Measure with enough arithmetic per byte, or a working set that fits in cache.

## Exercise

`TODO(unit-18)` in `pick()`: force `return poly_scalar;` and re-run. Case B should collapse to Case A. That is the dispatcher doing the work, not a different `-O` level.

Then restore the real `pick()`, and in the matrix section try `tile = 1` (same as Case D) vs `3` vs the L1-derived tile. If `3` already matches L1, you are seeing spatial locality (Unit 03), not a wrong parse — read the printed KiB anyway.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-18`).
