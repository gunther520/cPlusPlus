#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

constexpr int kN = 1 << 20;
constexpr int kSamples = 50;
constexpr int kWarmup = 5;

// noinline: if these are inlined, the compiler sees three distinct
// std::vector buffers and vectorizes both cases the same way.
__attribute__((noinline)) static void add_maybe_alias(float* a, float* b,
                                                      float* c, int n) {
  for (int i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

__attribute__((noinline)) static void add_restrict(float* __restrict__ a,
                                                   float* __restrict__ b,
                                                   float* __restrict__ c,
                                                   int n) {
  for (int i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

static float sum_branchy(float const* a, std::uint8_t const* mask, int n) {
  float s = 0.f;
  for (int i = 0; i < n; ++i) {
    if (mask[i]) {
      s += a[i];
    }
  }
  return s;
}

static float sum_branchless(float const* a, std::uint8_t const* mask, int n) {
  float s = 0.f;
  for (int i = 0; i < n; ++i) {
    s += a[i] * static_cast<float>(mask[i]);
  }
  return s;
}

int main() {
  ll::print_header("Unit 11 — vectorization");
  std::cout << "N=" << kN << "\n\n";

  std::vector<float> a(static_cast<std::size_t>(kN), 1.0f);
  std::vector<float> b(static_cast<std::size_t>(kN), 2.0f);
  std::vector<float> c(static_cast<std::size_t>(kN), 0.0f);

  std::mt19937 rng(7);
  std::bernoulli_distribution coin(0.5);
  std::vector<std::uint8_t> mask(static_cast<std::size_t>(kN));
  for (auto& m : mask) {
    m = static_cast<std::uint8_t>(coin(rng));
  }

  auto alias = ll::bench(
      [&] {
        // TODO(unit-11): try add_maybe_alias(a.data(), a.data(), c.data(), kN)
        add_maybe_alias(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case A (add, may alias)", alias);

  auto rest = ll::bench(
      [&] {
        add_restrict(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case B (add, __restrict__)", rest);
  ll::print_speedup("may alias", alias, "restrict", rest);

  std::cout << "\n";
  auto br = ll::bench(
      [&] {
        float s = sum_branchy(a.data(), mask.data(), kN);
        ll::do_not_optimize(s);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (branchy masked sum)", br);

  auto bl = ll::bench(
      [&] {
        float s = sum_branchless(a.data(), mask.data(), kN);
        ll::do_not_optimize(s);
      },
      kSamples, kWarmup);
  ll::print_stats("case D (branchless masked sum)", bl);
  ll::print_speedup("branchy sum", br, "branchless sum", bl);

  std::cout << "\nRebuild with OPT=3 and inspect simd in the asm.\n";
  return 0;
}
