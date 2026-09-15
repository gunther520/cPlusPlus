#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <emmintrin.h>
#define LL_HAS_SSE2 1
#else
#define LL_HAS_SSE2 0
#endif

constexpr int kN = 1 << 15;
constexpr int kSamples = 50;
constexpr int kWarmup = 5;

__attribute__((noinline, optimize("no-tree-vectorize"))) static void add_scalar(float const* a, float const* b,
                                                 float* c, int n) {
  for (int i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

#if LL_HAS_SSE2
// TODO(unit-15): process 8 floats per iteration (two __m128).
__attribute__((noinline)) static void add_sse2(float const* a, float const* b,
                                               float* c, int n) {
  int i = 0;
  for (; i + 4 <= n; i += 4) {
    __m128 va = _mm_loadu_ps(a + i);
    __m128 vb = _mm_loadu_ps(b + i);
    _mm_storeu_ps(c + i, _mm_add_ps(va, vb));
  }
  for (; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}
#endif

int main() {
  ll::print_header("Unit 15 — SIMD intrinsics");
  std::cout << "N=" << kN << " floats\n\n";

  std::vector<float> a(static_cast<std::size_t>(kN), 1.0f);
  std::vector<float> b(static_cast<std::size_t>(kN), 2.0f);
  std::vector<float> c(static_cast<std::size_t>(kN), 0.0f);

  auto scalar = ll::bench(
      [&] {
        add_scalar(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case A (scalar, noinline)", scalar);

#if LL_HAS_SSE2
  auto sse = ll::bench(
      [&] {
        add_sse2(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case B (SSE2 _mm_add_ps)", sse);
  ll::print_speedup("scalar", scalar, "SSE2", sse);
#else
  std::cout << "case B skipped: SSE2 not available on this CPU.\n";
#endif

  auto maybe_auto = ll::bench(
      [&] {
        float* cc = c.data();
        float const* aa = a.data();
        float const* bb = b.data();
        for (int i = 0; i < kN; ++i) {
          cc[i] = aa[i] + bb[i];
        }
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case C (plain loop, compiler may auto-vec)", maybe_auto);
  ll::print_speedup("scalar", scalar, "plain loop", maybe_auto);

  std::cout << "\nIntrinsics matter when the compiler will not vectorize.\n";
  return 0;
}
