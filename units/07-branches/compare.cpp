#include "bench.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

constexpr int kN = 1 << 20;
constexpr int kSamples = 60;
constexpr int kWarmup = 6;
// TODO(unit-07): try threshold 2, 128, 250 on unsorted data.
constexpr int kThreshold = 128;

// Opaque so the compiler cannot turn the if into cmov/SIMD. A real
// unpredictable branch to an extra call is what we want to measure.
__attribute__((noinline)) static void hit(std::uint64_t& c, std::uint8_t v) {
  c += v;
}

static std::uint64_t count_branchy(std::uint8_t const* p, int n, int thr) {
  std::uint64_t c = 0;
  for (int i = 0; i < n; ++i) {
    if (p[i] > thr) {
      hit(c, p[i]);
    }
  }
  ll::do_not_optimize(c);
  return c;
}

static std::uint64_t count_branchless(std::uint8_t const* p, int n, int thr) {
  std::uint64_t c = 0;
  for (int i = 0; i < n; ++i) {
    // Always do the work; the multiply is a mask (0 or 1).
    c += static_cast<std::uint64_t>(p[i]) * static_cast<std::uint64_t>(p[i] > thr);
  }
  ll::do_not_optimize(c);
  return c;
}

int main() {
  ll::print_header("Unit 07 — branches");
  std::cout << "N=" << kN << "  threshold=" << kThreshold << "\n\n";

  std::mt19937 rng(42);
  std::uniform_int_distribution<int> dist(0, 255);
  std::vector<std::uint8_t> unsorted(static_cast<std::size_t>(kN));
  for (auto& x : unsorted) {
    x = static_cast<std::uint8_t>(dist(rng));
  }
  std::vector<std::uint8_t> sorted = unsorted;
  std::sort(sorted.begin(), sorted.end());

  auto u = ll::bench(
      [&] { count_branchy(unsorted.data(), kN, kThreshold); }, kSamples, kWarmup);
  ll::print_stats("case A (branchy, unsorted)", u);

  auto s = ll::bench(
      [&] { count_branchy(sorted.data(), kN, kThreshold); }, kSamples, kWarmup);
  ll::print_stats("case B (branchy, sorted)", s);
  ll::print_speedup("unsorted branchy", u, "sorted branchy", s);

  std::cout << "\n";
  auto bl = ll::bench(
      [&] { count_branchless(unsorted.data(), kN, kThreshold); }, kSamples,
      kWarmup);
  ll::print_stats("case C (branchless, unsorted)", bl);
  ll::print_speedup("unsorted branchy", u, "branchless", bl);

  std::cout << "\nSame data, different order: the predictor is the variable.\n";
  return 0;
}
