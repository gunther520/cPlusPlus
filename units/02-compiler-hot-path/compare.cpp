#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

constexpr int kIters = 200000;
constexpr int kSamples = 150;
constexpr int kWarmup = 15;
constexpr int kAddIters = 2000000;

static std::uint64_t burn_dead(std::uint32_t const* p, int n) {
  std::uint64_t sum = 0;
  for (int i = 0; i < n; ++i) {
    sum += p[i];
  }
  // Result is returned but the caller ignores it. At -O2 the whole
  // function can disappear if the compiler can prove nothing else uses it.
  return sum;
}

static std::uint64_t burn_live(std::uint32_t const* p, int n) {
  std::uint64_t sum = 0;
  for (int i = 0; i < n; ++i) {
    sum += p[i];
  }
  // TODO(unit-02): comment this line out, rebuild with OPT=2, watch Case B collapse.
  ll::do_not_optimize(sum);
  return sum;
}

__attribute__((noinline)) static int add_noinline(int a, int b) {
  return a + b;
}

__attribute__((always_inline)) inline int add_inline(int a, int b) {
  return a + b;
}

int main() {
  ll::print_header("Unit 02 — compiler hot path");
  std::cout << "loop iters = " << kIters << "\n\n";

  std::vector<std::uint32_t> data(static_cast<std::size_t>(kIters));
  for (int i = 0; i < kIters; ++i) {
    data[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(i + 1);
  }

  auto a = ll::bench(
      [&] {
        (void)burn_dead(data.data(), kIters);
      },
      kSamples, kWarmup);
  ll::print_stats("case A (result unused)", a);

  auto b = ll::bench(
      [&] {
        (void)burn_live(data.data(), kIters);
      },
      kSamples, kWarmup);
  ll::print_stats("case B (do_not_optimize)", b);
  ll::print_speedup("kept live (real work)", b, "unused result (DCE)", a);
  std::cout << "(at -O2 a ratio >> 1 means the unused loop was deleted)\n";

  std::cout << "\n";
  volatile int seed = 1;  // volatile so -O2 cannot constant-fold the whole add loop
  int x = seed;

  auto noinline_stats = ll::bench(
      [&] {
        int s = x;
        for (int i = 0; i < kAddIters; ++i) {
          s = add_noinline(s, i);
        }
        ll::do_not_optimize(s);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (noinline add)", noinline_stats);

  auto inline_stats = ll::bench(
      [&] {
        int s = x;
        for (int i = 0; i < kAddIters; ++i) {
          s = add_inline(s, i);
        }
        ll::do_not_optimize(s);
      },
      kSamples, kWarmup);
  ll::print_stats("case D (always_inline add)", inline_stats);
  ll::print_speedup("noinline", noinline_stats, "always_inline", inline_stats);

  std::cout << "\nRe-run with: make clean && make run-02 OPT=0  (then OPT=2)\n";
  return 0;
}
