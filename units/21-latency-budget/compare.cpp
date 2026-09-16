#include "bench.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

constexpr int kSteps = 1 << 16;
constexpr int kSamples = 25;
constexpr int kWarmup = 3;
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

__attribute__((noinline, optimize("no-tree-vectorize"))) static std::uint64_t scan(
    std::uint32_t const* a, int n) {
  std::uint64_t s = 0;
  for (int i = 0; i < n; ++i) {
    s += a[i];
  }
  return s;
}

static void report(char const* name, ll::Stats const& st, int hops) {
  ll::print_stats(name, st);
  double per = hops > 0 ? st.p50_ns / static_cast<double>(hops) : 0.0;
  double fit = per > 0.0 ? kBudgetNs / per : 0.0;
  std::cout << "    p50/hop=" << per << " ns   hops in " << kBudgetNs
            << " ns ≈ " << fit << '\n';
}

int main() {
  ll::print_header("Unit 21 — measured latency budget");
  std::cout << "dependent chase i = next[i], " << kSteps << " hops/sample\n";
  std::cout << "software envelope " << kBudgetNs
            << " ns (1.5 us SLO minus ~1 us kernel-bypass NIC)\n\n";

  struct Size {
    char const* name;
    int n;
  };
  Size sizes[] = {
      {"case A (16 KiB, L1-ish)", 16 * 1024 / 4},
      {"case B (256 KiB, L2-ish)", 256 * 1024 / 4},
      {"case C (64 MiB, LLC/DRAM-ish)", (64 * 1024 * 1024) / 4},
  };

  std::uint32_t seed = 1;
  std::vector<std::uint32_t> big;
  for (Size const& sz : sizes) {
    auto next = make_cycle(sz.n, seed++);
    auto st = ll::bench(
        [&] {
          std::uint32_t i = chase(next.data(), 0, kSteps);
          ll::do_not_optimize(i);
        },
        kSamples, kWarmup);
    report(sz.name, st, kSteps);
    if (sz.n == (64 * 1024 * 1024) / 4) {
      big = std::move(next);
    }
  }

  // TODO(unit-21): sequential scan of the 64 MiB buffer; compare ns/elem to case C.
  int n_big = static_cast<int>(big.size());
  auto seq = ll::bench(
      [&] {
        std::uint64_t s = scan(big.data(), n_big);
        ll::do_not_optimize(s);
      },
      12, 2);
  ll::print_stats("case C' (sequential 64 MiB)", seq);
  std::cout << "    p50/elem=" << (seq.p50_ns / static_cast<double>(n_big))
            << " ns  (chase hops are the budget; this is prefetch)\n";

  std::cout << "\nThe matcher budget is the chase column, not the sequential add.\n";
  return 0;
}
