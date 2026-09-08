// Shared micro-benchmark helpers for the low-latency C++ units.
// Measure p50/p99/p99.9, not just the mean. Force the compiler to keep work.

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#define LL_HAS_RDTSC 1
#else
#define LL_HAS_RDTSC 0
#endif

namespace ll {

inline void clobber_memory() {
  asm volatile("" ::: "memory");
}

// Prevent the compiler from deleting or inventing the value.
// Use this on results (and sometimes inputs) of every timed region.
template <typename T>
inline void do_not_optimize(T const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

template <typename T>
inline void do_not_optimize(T& value) {
#if defined(__clang__)
  asm volatile("" : "+r,m"(value) : : "memory");
#else
  asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

struct Stats {
  int samples = 0;
  double min_ns = 0;
  double p50_ns = 0;
  double p99_ns = 0;
  double p999_ns = 0;
  double mean_ns = 0;
  double stddev_ns = 0;
};

inline double percentile_ns(std::vector<double> const& sorted, double p) {
  if (sorted.empty()) {
    return 0.0;
  }
  double idx = p * static_cast<double>(sorted.size() - 1);
  std::size_t lo = static_cast<std::size_t>(idx);
  std::size_t hi = std::min(lo + 1, sorted.size() - 1);
  double frac = idx - static_cast<double>(lo);
  return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
}

inline Stats summarize(std::vector<double> samples_ns) {
  Stats s;
  s.samples = static_cast<int>(samples_ns.size());
  if (samples_ns.empty()) {
    return s;
  }
  std::sort(samples_ns.begin(), samples_ns.end());
  s.min_ns = samples_ns.front();
  s.p50_ns = percentile_ns(samples_ns, 0.50);
  s.p99_ns = percentile_ns(samples_ns, 0.99);
  s.p999_ns = percentile_ns(samples_ns, 0.999);
  s.mean_ns = std::accumulate(samples_ns.begin(), samples_ns.end(), 0.0) /
              static_cast<double>(samples_ns.size());
  double acc = 0.0;
  for (double v : samples_ns) {
    double d = v - s.mean_ns;
    acc += d * d;
  }
  s.stddev_ns = std::sqrt(acc / static_cast<double>(samples_ns.size()));
  return s;
}

enum class ClockKind { Steady, Rdtsc };

inline std::uint64_t read_rdtsc() {
#if LL_HAS_RDTSC
  _mm_mfence();
  return static_cast<std::uint64_t>(__rdtsc());
#else
  return 0;
#endif
}

// Rough cycles-to-ns calibration. Good enough to compare clocks, not for
// claiming picosecond accuracy.
inline double ns_per_cycle() {
#if !LL_HAS_RDTSC
  return 0.0;
#else
  using clock = std::chrono::steady_clock;
  auto t0 = clock::now();
  std::uint64_t c0 = read_rdtsc();
  auto target = t0 + std::chrono::milliseconds(20);
  while (clock::now() < target) {
  }
  std::uint64_t c1 = read_rdtsc();
  auto t1 = clock::now();
  double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
  double cycles = static_cast<double>(c1 - c0);
  return cycles > 0.0 ? ns / cycles : 0.0;
#endif
}

template <typename Fn>
Stats bench(Fn&& fn, int samples = 100, int warmup = 10,
            ClockKind clock = ClockKind::Steady) {
  for (int i = 0; i < warmup; ++i) {
    fn();
  }

  std::vector<double> times;
  times.reserve(static_cast<std::size_t>(samples));

  double cyc2ns = (clock == ClockKind::Rdtsc) ? ns_per_cycle() : 0.0;

  for (int i = 0; i < samples; ++i) {
    if (clock == ClockKind::Rdtsc && LL_HAS_RDTSC) {
      std::uint64_t a = read_rdtsc();
      fn();
      std::uint64_t b = read_rdtsc();
      times.push_back(static_cast<double>(b - a) * cyc2ns);
    } else {
      auto a = std::chrono::steady_clock::now();
      fn();
      auto b = std::chrono::steady_clock::now();
      times.push_back(std::chrono::duration<double, std::nano>(b - a).count());
    }
  }
  return summarize(std::move(times));
}

inline void print_ns(double ns) {
  std::cout << std::fixed << std::setprecision(1);
  if (ns >= 1e6) {
    std::cout << (ns / 1e6) << " ms";
  } else if (ns >= 1e3) {
    std::cout << (ns / 1e3) << " us";
  } else {
    std::cout << ns << " ns";
  }
}

inline void print_stats(char const* name, Stats const& s) {
  std::cout << std::left << std::setw(36) << name << "  ";
  std::cout << "n=" << s.samples << "  min=";
  print_ns(s.min_ns);
  std::cout << "  p50=";
  print_ns(s.p50_ns);
  std::cout << "  p99=";
  print_ns(s.p99_ns);
  std::cout << "  p99.9=";
  print_ns(s.p999_ns);
  std::cout << "  mean=";
  print_ns(s.mean_ns);
  std::cout << '\n';
}

inline void print_speedup(char const* slow_name, Stats const& slow,
                          char const* fast_name, Stats const& fast) {
  auto ratio = [](double a, double b) {
    return b > 0.0 ? a / b : 0.0;
  };
  std::cout << std::setprecision(2);
  std::cout << "speedup " << fast_name << " vs " << slow_name
            << ":  p50=" << ratio(slow.p50_ns, fast.p50_ns) << "x"
            << "  p99=" << ratio(slow.p99_ns, fast.p99_ns) << "x\n";
}

inline void print_header(char const* unit) {
  std::cout << "\n=== " << unit << " ===\n";
#if defined(__OPTIMIZE__)
  std::cout << "compiled with optimization macros on (__OPTIMIZE__)\n";
#else
  std::cout << "compiled WITHOUT optimizer (__OPTIMIZE__ undefined, likely -O0)\n";
#endif
}

}  // namespace ll
