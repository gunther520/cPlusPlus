#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

constexpr int kN = 150000;
constexpr int kAdd = 1500000;

static std::uint64_t work(std::uint32_t const* p, int n) {
  std::uint64_t s = 0;
  for (int i = 0; i < n; ++i) {
    s += p[i];
  }
  return s;
}

__attribute__((noinline)) static int add_noinline(int a, int b) { return a + b; }
__attribute__((always_inline)) inline int add_inline(int a, int b) { return a + b; }

int main() {
  ll::print_header("Unit 02 lab");
  std::vector<std::uint32_t> data(static_cast<std::size_t>(kN), 1);

  auto dead = ll::bench([&] { (void)work(data.data(), kN); }, 80, 8);
  ll::print_stats("case A (unused result)", dead);

  auto live = ll::bench(
      [&] {
        auto s = work(data.data(), kN);
        // TODO: ll::do_not_optimize(s);  — without this, -O2 may still delete work
        (void)s;
      },
      80, 8);
  ll::print_stats("case B (TODO: keep live)", live);

  volatile int seed = 1;
  int x = seed;
  auto noinline_s = ll::bench(
      [&] {
        int s = x;
        for (int i = 0; i < kAdd; ++i) {
          s = add_noinline(s, 1);
        }
        ll::do_not_optimize(s);
      },
      40, 5);
  ll::print_stats("noinline add", noinline_s);

  auto inline_s = ll::bench(
      [&] {
        int s = x;
        for (int i = 0; i < kAdd; ++i) {
          s = add_inline(s, 1);
        }
        ll::do_not_optimize(s);
      },
      40, 5);
  ll::print_stats("always_inline add", inline_s);
  ll::print_speedup("noinline", noinline_s, "inline", inline_s);
  return 0;
}
