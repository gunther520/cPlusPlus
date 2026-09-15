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

// Modest N plus a few Horner steps: SIMD vs SIMD is otherwise DRAM (Unit 15).
constexpr int kN = 1 << 15;
constexpr int kRows = 1024;
constexpr int kCols = 1024;
constexpr int kSamples = 40;
constexpr int kWarmup = 4;

using PolyFn = void (*)(float const*, float const*, float*, int);

// y = a+b, then four Horner steps y = y*a + b. Enough ALU that 256-bit can win.
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

// Compiled with AVX2 even though the file is not built with -mavx2.
// Only this function (and callers that themselves have target("avx2")) may use ymm.
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
#if LL_X86
  __builtin_cpu_init();
  // TODO(unit-18): return poly_scalar; and re-run — Case B should match A.
  if (__builtin_cpu_supports("avx2")) {
    return poly_avx2;
  }
  if (__builtin_cpu_supports("sse2")) {
    return poly_sse2;
  }
#endif
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
  for (int i = 0; i < 8; ++i) {
    std::string const base =
        "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i);
    std::ifstream type(base + "/type");
    std::ifstream level(base + "/level");
    std::ifstream size(base + "/size");
    if (!type || !level || !size) {
      continue;
    }
    std::string t;
    std::string lv;
    std::string sz;
    std::getline(type, t);
    std::getline(level, lv);
    std::getline(size, sz);
    if (t == "Data" && lv == "1") {
      std::size_t n = parse_sysfs_bytes(sz);
      if (n > 0) {
        return n;
      }
    }
  }
  return 32 * 1024;
}

// Strip width so rows * tile * sizeof(float) is about half of L1d.
static int l1_col_tile(int rows) {
  int bytes = static_cast<int>(read_l1d_bytes() / 2);
  int denom = std::max(rows, 1) * static_cast<int>(sizeof(float));
  int b = bytes / denom;
  return std::max(4, std::min(b, 64));
}

// Column-major walk: inner index strides by kCols floats (Unit 03).
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

// Same sum, blocked so the inner walk is consecutive floats (a cache line).
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
  ll::print_header("Unit 18 — hardware-adaptive kernels");

#if LL_X86
  __builtin_cpu_init();
  std::cout << "cpu: sse2=" << (__builtin_cpu_supports("sse2") ? 1 : 0)
            << " avx2=" << (__builtin_cpu_supports("avx2") ? 1 : 0) << '\n';
#else
  std::cout << "cpu: not x86; SIMD kernels compiled out\n";
#endif

  std::size_t l1d = read_l1d_bytes();
  int tile = l1_col_tile(kRows);
  std::cout << "L1d=" << (l1d / 1024) << " KiB  (col tile=" << tile << ")\n";

  PolyFn dispatched = pick();
  std::cout << "picked poly: " << kernel_name(dispatched) << "\n\n";

  std::vector<float> a(static_cast<std::size_t>(kN), 1.0f);
  std::vector<float> b(static_cast<std::size_t>(kN), 2.0f);
  std::vector<float> c(static_cast<std::size_t>(kN), 0.0f);

  auto scalar = ll::bench(
      [&] {
        poly_scalar(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case A (scalar poly)", scalar);

  auto picked = ll::bench(
      [&] {
        dispatched(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case B (runtime pick)", picked);
  ll::print_speedup("scalar", scalar, "picked", picked);

#if LL_X86
  auto sse = ll::bench(
      [&] {
        poly_sse2(a.data(), b.data(), c.data(), kN);
        ll::do_not_optimize(c.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case C (always SSE2)", sse);
  ll::print_speedup("sse2", sse, "picked", picked);
#else
  std::cout << "case C skipped: SSE2 not available on this CPU.\n";
#endif

  std::vector<float> mat(static_cast<std::size_t>(kRows) * static_cast<std::size_t>(kCols),
                         1.0f);
  std::cout << "\nmatrix " << kRows << "x" << kCols << " column walk vs L1 tile\n";

  auto strided = ll::bench(
      [&] {
        float acc = col_sum(mat.data(), kRows, kCols);
        ll::do_not_optimize(acc);
      },
      16, 2);
  ll::print_stats("case D (column-major)", strided);

  auto blocked = ll::bench(
      [&] {
        float acc = col_sum_blocked(mat.data(), kRows, kCols, tile);
        ll::do_not_optimize(acc);
      },
      16, 2);
  ll::print_stats("case E (L1-derived tile)", blocked);
  ll::print_speedup("column", strided, "L1 tile", blocked);

  std::cout << "\nDetect once at startup. Do not CPUID in the tick.\n";
  return 0;
}
