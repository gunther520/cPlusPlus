#include "bench.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/mman.h>
#include <vector>

// -----------------------------------------------------------------------------
// This file does not compare two algorithms. It compares two *ways of timing*
// the same dummy workload (`burn`), and it shows when warmup does nothing.
//
// Why kWarmup looks useless after filling `data`:
//   the fill loop already paid page faults and filled the cache. The CPU is hot.
//
// Case A: fresh mmap, first burn (cold: minor page faults inside the timed region).
// Case B: warmup + 200 samples on the already-filled buffer (hot p50/p99).
// Case C: same as B but the stopwatch is rdtsc.
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

// Untimed hot-path calls before we record Case B/C. Only matters if the
// pages are not already resident (see map_fresh).
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

static std::size_t work_bytes() {
  return static_cast<std::size_t>(kWork) * sizeof(std::uint32_t);
}

// Anonymous pages: not backed by RAM until the first load/store (minor fault).
static std::uint32_t* map_fresh() {
  void* p = mmap(nullptr, work_bytes(), PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (p == MAP_FAILED) {
    std::perror("mmap");
    std::exit(1);
  }
  return static_cast<std::uint32_t*>(p);
}

static void unmap_fresh(std::uint32_t* p) {
  munmap(p, work_bytes());
}

int main() {
  ll::print_header("Unit 01 — measure first");
  std::cout << "inner work = " << kWork << " mixed loads ("
            << (static_cast<double>(work_bytes()) / (1024.0 * 1024.0))
            << " MiB)\n\n";

  std::vector<std::uint32_t> data(static_cast<std::size_t>(kWork));
  for (int i = 0; i < kWork; ++i) {
    data[static_cast<std::size_t>(i)] =
        static_cast<std::uint32_t>(i * kLcgMul + kLcgAdd);
  }

  std::cout << "Already-hot: fill() already faulted and walked every page.\n";
  std::cout << "A warmup loop here would not move the number.\n";
  for (int r = 0; r < 3; ++r) {
    auto a = std::chrono::steady_clock::now();
    burn(data.data(), kWork);
    auto b = std::chrono::steady_clock::now();
    double ns = std::chrono::duration<double, std::nano>(b - a).count();
    std::cout << "  hot run " << (r + 1) << ": ";
    ll::print_ns(ns);
    std::cout << '\n';
  }

  std::cout << "\nCase A: new mmap each time; first burn faults the pages in\n";
  for (int r = 0; r < kOneShotRuns; ++r) {
    std::uint32_t* fresh = map_fresh();
    auto a = std::chrono::steady_clock::now();
    burn(fresh, kWork);
    auto b = std::chrono::steady_clock::now();
    unmap_fresh(fresh);
    double ns = std::chrono::duration<double, std::nano>(b - a).count();
    std::cout << "  cold first-touch " << (r + 1) << ": ";
    ll::print_ns(ns);
    std::cout << '\n';
  }

  std::cout << "\nCase B: warmup " << kWarmup
            << " untimed burns on `data`, then percentiles (hot)\n";
  auto chrono_stats = ll::bench([&] { burn(data.data(), kWork); }, kSamples,
                                kWarmup, ll::ClockKind::Steady);
  ll::print_stats("case B (chrono + percentiles)", chrono_stats);

#if LL_HAS_RDTSC
  std::cout << "\nCase C: same hot work timed with rdtsc (calibrated to ns)\n";
  auto rdtsc_stats = ll::bench([&] { burn(data.data(), kWork); }, kSamples,
                               kWarmup, ll::ClockKind::Rdtsc);
  ll::print_stats("case C (rdtsc + percentiles)", rdtsc_stats);
  ll::print_speedup("chrono", chrono_stats, "rdtsc", rdtsc_stats);
  std::cout << "(ratio near 1.0 means the two clocks agree on this work)\n";
#else
  std::cout << "\nCase C skipped: rdtsc not available on this CPU.\n";
#endif

  std::cout << "\nCase A (first touch) should sit well above Case B p50.\n";
  std::cout << "That gap is the one-time cost warmup is meant to skip.\n";
  return 0;
}
