#include "bench.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#define LL_X86 1
#else
#define LL_X86 0
#endif

constexpr int kN = 1 << 15;
constexpr int kRows = 1024;
constexpr int kCols = 1024;

using PolyFn = void (*)(float const*, float const*, float*, int);

__attribute__((noinline, optimize("no-tree-vectorize"))) static void poly_scalar(
    float const* a, float const* b, float* c, int n) {
  for (int i = 0; i < n; ++i) {
    float y = a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    c[i] = y;
  }
}

#if LL_X86
__attribute__((target("sse2"), noinline)) static void poly_sse2(float const* a,
                                                                float const* b,
                                                                float* c, int n) {
  int i = 0;
  for (; i + 4 <= n; i += 4) {
    __m128 va = _mm_loadu_ps(a + i);
    __m128 vb = _mm_loadu_ps(b + i);
    __m128 y = _mm_add_ps(va, vb);
    y = _mm_add_ps(_mm_mul_ps(y, va), vb);
    y = _mm_add_ps(_mm_mul_ps(y, va), vb);
    y = _mm_add_ps(_mm_mul_ps(y, va), vb);
    y = _mm_add_ps(_mm_mul_ps(y, va), vb);
    _mm_storeu_ps(c + i, y);
  }
  for (; i < n; ++i) {
    float y = a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    c[i] = y;
  }
}

__attribute__((target("avx2"), noinline)) static void poly_avx2(float const* a,
                                                                float const* b,
                                                                float* c, int n) {
  int i = 0;
  for (; i + 8 <= n; i += 8) {
    __m256 va = _mm256_loadu_ps(a + i);
    __m256 vb = _mm256_loadu_ps(b + i);
    __m256 y = _mm256_add_ps(va, vb);
    y = _mm256_add_ps(_mm256_mul_ps(y, va), vb);
    y = _mm256_add_ps(_mm256_mul_ps(y, va), vb);
    y = _mm256_add_ps(_mm256_mul_ps(y, va), vb);
    y = _mm256_add_ps(_mm256_mul_ps(y, va), vb);
    _mm256_storeu_ps(c + i, y);
  }
  for (; i < n; ++i) {
    float y = a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    y = y * a[i] + b[i];
    c[i] = y;
  }
  _mm256_zeroupper();
}
#endif

static PolyFn pick() {
  // TODO: __builtin_cpu_init(); then avx2 / sse2 / scalar. Not this:
  return poly_scalar;
}

static char const* kernel_name(PolyFn fn) {
#if LL_X86
  if (fn == poly_avx2) {
    return "avx2";
  }
  if (fn == poly_sse2) {
    return "sse2";
  }
#endif
  (void)fn;
  return "scalar";
}

static std::size_t parse_sysfs_bytes(std::string s) {
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ')) {
    s.pop_back();
  }
  if (s.empty()) {
    return 0;
  }
  std::size_t mul = 1;
  char u = s.back();
  if (u == 'K' || u == 'k') {
    mul = 1024;
    s.pop_back();
  } else if (u == 'M' || u == 'm') {
    mul = 1024 * 1024;
    s.pop_back();
  }
  try {
    return static_cast<std::size_t>(std::stoul(s)) * mul;
  } catch (...) {
    return 0;
  }
}

static std::size_t read_l1d_bytes() {
  // TODO: walk cpu0/cache/index* for type=Data, level=1, parse size (e.g. 48K).
  (void)parse_sysfs_bytes;
  return 32 * 1024;
}

static int l1_col_tile(int rows) {
  // TODO: ~half of L1d / (rows * sizeof(float)), clamp to [4, 64]. Not 3.
  (void)rows;
  (void)read_l1d_bytes;
  return 3;
}

__attribute__((noinline, optimize("no-tree-vectorize"))) static float col_sum(
    float const* m, int rows, int cols) {
  float acc = 0.f;
  for (int j = 0; j < cols; ++j) {
    for (int i = 0; i < rows; ++i) {
      acc += m[i * cols + j];
    }
  }
  return acc;
}

__attribute__((noinline, optimize("no-tree-vectorize"))) static float col_sum_blocked(
    float const* m, int rows, int cols, int tile) {
  tile = std::max(tile, 1);
  float acc = 0.f;
  for (int jj = 0; jj < cols; jj += tile) {
    int const j_end = std::min(jj + tile, cols);
    for (int i = 0; i < rows; ++i) {
      for (int j = jj; j < j_end; ++j) {
        acc += m[i * cols + j];
      }
    }
  }
  return acc;
}

int main() {
  ll::print_header("Unit 18 lab");

  PolyFn dispatched = pick();
  std::cout << "picked poly: " << kernel_name(dispatched)
            << "  L1d=" << (read_l1d_bytes() / 1024)
            << " KiB  col_tile=" << l1_col_tile(kRows) << '\n';

  std::vector<float> a(static_cast<std::size_t>(kN), 1.f);
  std::vector<float> b(static_cast<std::size_t>(kN), 2.f);
  std::vector<float> c(static_cast<std::size_t>(kN), 0.f);

  auto sc = ll::bench(
      [&] {
        poly_scalar(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      40, 4);
  ll::print_stats("case A (scalar poly)", sc);

  auto picked = ll::bench(
      [&] {
        dispatched(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      40, 4);
  ll::print_stats("case B (TODO: pick ISA)", picked);
  ll::print_speedup("scalar", sc, "picked", picked);

  std::vector<float> mat(static_cast<std::size_t>(kRows) * static_cast<std::size_t>(kCols),
                         1.f);

  auto strided = ll::bench(
      [&] {
        float acc = col_sum(mat.data(), kRows, kCols);
        ll::do_not_optimize(acc);
      },
      12, 1);
  ll::print_stats("case C (column-major)", strided);

  auto blocked = ll::bench(
      [&] {
        float acc = col_sum_blocked(mat.data(), kRows, kCols, l1_col_tile(kRows));
        ll::do_not_optimize(acc);
      },
      12, 1);
  ll::print_stats("case D (TODO: L1 col tile)", blocked);
  ll::print_speedup("column", strided, "blocked", blocked);
  return 0;
}
