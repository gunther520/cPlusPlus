#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

constexpr int kN = 1 << 20;

static float sum_branchy(float const* a, std::uint8_t const* m, int n) {
  float s = 0.f;
  for (int i = 0; i < n; ++i) {
    if (m[i]) {
      s += a[i];
    }
  }
  return s;
}

int main() {
  ll::print_header("Unit 11 lab");
  std::vector<float> a(static_cast<std::size_t>(kN), 1.f);
  std::mt19937 rng(3);
  std::bernoulli_distribution coin(0.5);
  std::vector<std::uint8_t> mask(static_cast<std::size_t>(kN));
  for (auto& x : mask) {
    x = static_cast<std::uint8_t>(coin(rng));
  }

  auto br = ll::bench(
      [&] {
        float s = sum_branchy(a.data(), mask.data(), kN);
        ll::do_not_optimize(s);
      },
      30, 3);
  ll::print_stats("case A (branchy mask)", br);

  auto bl = ll::bench(
      [&] {
        float s = 0.f;
        for (int i = 0; i < kN; ++i) {
          // TODO: s += a[i] * (float)mask[i];  no if
          if (mask[static_cast<std::size_t>(i)]) {
            s += a[static_cast<std::size_t>(i)];
          }
        }
        ll::do_not_optimize(s);
      },
      30, 3);
  ll::print_stats("case B (TODO: branchless)", bl);
  ll::print_speedup("branchy", br, "branchless", bl);
  return 0;
}
