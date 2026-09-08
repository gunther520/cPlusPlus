#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

// -----------------------------------------------------------------------------
// This file does not compare two algorithms. It compares two *ways of timing*
// the same dummy workload (`burn`).
//
// Case A: eight raw start/stop clocks (what beginners do).
// Case B: warmup + 200 samples, then p50/p99 (what you actually want).
// Case C: same as B but the stopwatch is the CPU cycle counter (rdtsc).
// -----------------------------------------------------------------------------

// How many array elements `burn` walks. Sized so one call is hundreds of
// microseconds: big enough that we are not measuring the timer (~20 ns),
// small enough that 200 samples finish instantly.
// TODO(unit-01): change kWork by 10x and watch p50 vs p99/p50.
constexpr int kWork = 250000;

// How many isolated "start, work, stop, print" lines for Case A.
constexpr int kOneShotRuns = 8;

// After warmup, how many timed calls go into the percentile table.
// p99 of 200 samples is roughly the 198th fastest — you need a crowd
// of samples or "p99" is just your single worst run.
constexpr int kSamples = 200;

// Untimed calls before we start recording. Pays for page faults, I-cache,
// and the CPU coming out of idle. Those belong in startup, not in p50.
constexpr int kWarmup = 20;

// Golden-ratio bits: floor(2^64 / φ) and floor(2^32 / φ). Used in Knuth /
// Fibonacci hashing. Any "ugly" odd constant would do; these are traditional.
constexpr std::uint64_t kMix64 = 0x9e3779b97f4a7c15ull;
constexpr std::uint64_t kMix32 = 0x9e3779b9ull;

// glibc `rand()` LCG: x = x * 1103515245 + 12345. Fills `data` with a
// cheap non-linear pattern so the compiler cannot assume data[i] == i
// and delete the loop.
constexpr std::uint32_t kLcgMul = 1103515245u;
constexpr std::uint32_t kLcgAdd = 12345u;

// Dummy "work": load every element and mix it into `sum`.
// The mix (xor, add, shift) is not a good hash. It exists so -O2 cannot
// replace the loop with a closed-form formula (Unit 02 will show that trick).
static std::uint64_t burn(std::uint32_t const* p, int n) {
  std::uint64_t sum = kMix64;
  for (int i = 0; i < n; ++i) {
    sum ^= static_cast<std::uint64_t>(p[i]) + kMix32 + (sum << 6) + (sum >> 2);
  }
  // Pretend `sum` is observed (sent on a wire, stored, printed).
  // Without this, -O2 may delete the entire loop.
  ll::do_not_optimize(sum);
  return sum;
}

int main() {
  ll::print_header("Unit 01 — measure first");
  std::cout << "inner work = " << kWork << " mixed loads\n\n";

  std::vector<std::uint32_t> data(static_cast<std::size_t>(kWork));
  for (int i = 0; i < kWork; ++i) {
    data[static_cast<std::size_t>(i)] =
        static_cast<std::uint32_t>(i * kLcgMul + kLcgAdd);
  }

  std::cout << "Case A: one-shot std::chrono (no warmup, 8 independent timings)\n";
  for (int r = 0; r < kOneShotRuns; ++r) {
    auto a = std::chrono::steady_clock::now();
    burn(data.data(), kWork);
    auto b = std::chrono::steady_clock::now();
    double ns = std::chrono::duration<double, std::nano>(b - a).count();
    std::cout << "  run " << (r + 1) << ": ";
    ll::print_ns(ns);
    std::cout << '\n';
  }

  std::cout << "\nCase B: warmed percentile bench, steady_clock\n";
  auto chrono_stats = ll::bench([&] { burn(data.data(), kWork); }, kSamples,
                                kWarmup, ll::ClockKind::Steady);
  ll::print_stats("case B (chrono + percentiles)", chrono_stats);

#if LL_HAS_RDTSC
  std::cout << "\nCase C: same work timed with rdtsc (calibrated to ns)\n";
  auto rdtsc_stats = ll::bench([&] { burn(data.data(), kWork); }, kSamples,
                               kWarmup, ll::ClockKind::Rdtsc);
  ll::print_stats("case C (rdtsc + percentiles)", rdtsc_stats);
  ll::print_speedup("chrono", chrono_stats, "rdtsc", rdtsc_stats);
  std::cout << "(ratio near 1.0 means the two clocks agree on this work)\n";
#else
  std::cout << "\nCase C skipped: rdtsc not available on this CPU.\n";
#endif

  std::cout << "\nRead OBJECTIVES.md: one-shot scatter vs p50/p99 is the lesson.\n";
  return 0;
}
