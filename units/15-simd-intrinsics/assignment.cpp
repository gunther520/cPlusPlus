#include "bench.hpp"

#include <iostream>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <emmintrin.h>
#define LL_HAS_SSE2 1
#else
#define LL_HAS_SSE2 0
#endif

constexpr int kN = 1 << 15;

__attribute__((noinline, optimize("no-tree-vectorize"))) static void add_scalar(
    float const* a, float const* b, float* c, int n) {
  for (int i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

int main() {
  ll::print_header("Unit 15 lab");
  std::vector<float> a(static_cast<std::size_t>(kN), 1.f);
  std::vector<float> b(static_cast<std::size_t>(kN), 2.f);
  std::vector<float> c(static_cast<std::size_t>(kN), 0.f);

  auto sc = ll::bench(
      [&] {
        add_scalar(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      40, 4);
  ll::print_stats("case A (scalar)", sc);

#if LL_HAS_SSE2
  auto sse = ll::bench(
      [&] {
        // TODO: SSE2 add in chunks of 4, scalar tail
        add_scalar(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      40, 4);
  ll::print_stats("case B (TODO: SSE2)", sse);
  ll::print_speedup("scalar", sc, "sse", sse);
#else
  std::cout << "SSE2 not available; scalar-only lab.\n";
#endif
  return 0;
}
