#include "bench.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

constexpr int kSteps = 1 << 16;
constexpr double kBudgetNs = 500.0;

static std::vector<std::uint32_t> make_cycle(int n, std::uint32_t seed) {
  std::vector<std::uint32_t> idx(static_cast<std::size_t>(n));
  std::vector<std::uint32_t> next(static_cast<std::size_t>(n));
  std::iota(idx.begin(), idx.end(), 0);
  std::mt19937 rng(seed);
  std::shuffle(idx.begin(), idx.end(), rng);
  for (int i = 0; i < n; ++i) {
    next[idx[static_cast<std::size_t>(i)]] =
        idx[static_cast<std::size_t>((i + 1) % n)];
  }
  return next;
}

__attribute__((noinline)) static std::uint32_t chase(std::uint32_t const* next,
                                                     std::uint32_t start, int steps) {
  std::uint32_t i = start;
  for (int t = 0; t < steps; ++t) {
    i = next[i];
  }
  return i;
}

static void report(char const* name, ll::Stats const& st) {
  ll::print_stats(name, st);
  double per = st.p50_ns / static_cast<double>(kSteps);
  std::cout << "    p50/hop=" << per << " ns   hops in " << kBudgetNs
            << " ns ≈ " << (per > 0.0 ? kBudgetNs / per : 0.0) << '\n';
}

int main() {
  ll::print_header("Unit 21 lab");

  auto small = make_cycle(16 * 1024 / 4, 1);
  auto a = ll::bench(
      [&] {
        std::uint32_t i = chase(small.data(), 0, kSteps);
        ll::do_not_optimize(i);
      },
      20, 2);
  report("case A (16 KiB chase)", a);

  auto b = ll::bench(
      [&] {
        // TODO: 64 MiB cycle, then chase. Not this 16 KiB buffer.
        std::uint32_t i = chase(small.data(), 0, kSteps);
        ll::do_not_optimize(i);
      },
      20, 2);
  report("case B (TODO: 64 MiB chase)", b);
  ll::print_speedup("far", b, "L1", a);
  return 0;
}
