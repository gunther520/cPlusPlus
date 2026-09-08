#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

// TODO(unit-01): change kWork by 10x and watch p50 vs p99/p50.
constexpr int kWork = 250000;
constexpr int kOneShotRuns = 8;
constexpr int kSamples = 200;
constexpr int kWarmup = 20;

static std::uint64_t burn(std::uint32_t const* p, int n) {
  std::uint64_t sum = 0x9e3779b97f4a7c15ull;
  for (int i = 0; i < n; ++i) {
    sum ^= static_cast<std::uint64_t>(p[i]) + 0x9e3779b9ull + (sum << 6) +
           (sum >> 2);
  }
  ll::do_not_optimize(sum);
  return sum;
}

int main() {
  ll::print_header("Unit 01 — measure first");
  std::cout << "inner work = " << kWork << " mixed loads\n\n";

  std::vector<std::uint32_t> data(static_cast<std::size_t>(kWork));
  for (int i = 0; i < kWork; ++i) {
    data[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(i * 1103515245u + 12345u);
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
