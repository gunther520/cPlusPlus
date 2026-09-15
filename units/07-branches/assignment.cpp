#include "bench.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

constexpr int kN = 1 << 20;
constexpr int kThreshold = 128;

__attribute__((noinline)) static void hit(std::uint64_t& c, std::uint8_t v) {
  c += v;
}

static std::uint64_t branchy(std::uint8_t const* p, int n, int thr) {
  std::uint64_t c = 0;
  for (int i = 0; i < n; ++i) {
    if (p[i] > thr) {
      hit(c, p[i]);
    }
  }
  ll::do_not_optimize(c);
  return c;
}

int main() {
  ll::print_header("Unit 07 lab");
  std::mt19937 rng(1);
  std::uniform_int_distribution<int> d(0, 255);
  std::vector<std::uint8_t> u(static_cast<std::size_t>(kN));
  for (auto& x : u) {
    x = static_cast<std::uint8_t>(d(rng));
  }
  auto sorted = u;
  std::sort(sorted.begin(), sorted.end());

  auto a = ll::bench([&] { branchy(u.data(), kN, kThreshold); }, 40, 4);
  ll::print_stats("case A (branchy unsorted)", a);
  auto b = ll::bench([&] { branchy(sorted.data(), kN, kThreshold); }, 40, 4);
  ll::print_stats("case B (branchy sorted)", b);

  auto c = ll::bench(
      [&] {
        std::uint64_t s = 0;
        for (int i = 0; i < kN; ++i) {
          // TODO: branchless using (u[i] > kThreshold)
          if (u[static_cast<std::size_t>(i)] > kThreshold) {
            s += u[static_cast<std::size_t>(i)];
          }
        }
        ll::do_not_optimize(s);
      },
      40, 4);
  ll::print_stats("case C (TODO: branchless)", c);
  std::cout << "Try kThreshold 2 and 250 in ASSIGNMENT.md\n";
  return 0;
}
