#include "bench.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

constexpr std::size_t kBytes = 64ull << 20;
constexpr int kTouches = 1 << 18;
constexpr int kSamplesCold = 8;
constexpr int kSamplesHot = 20;
constexpr int kWarmup = 2;

static int n_elems() {
  return static_cast<int>(kBytes / sizeof(std::uint32_t));
}

static int page_stride_elems() {
  long pg = sysconf(_SC_PAGESIZE);
  if (pg <= 0) {
    pg = 4096;
  }
  return static_cast<int>(pg / static_cast<long>(sizeof(std::uint32_t)));
}

__attribute__((noinline, optimize("no-tree-vectorize"))) static std::uint64_t walk(
    std::uint32_t const* a, int n, int stride, int touches) {
  std::uint64_t sum = 0;
  int i = 0;
  for (int t = 0; t < touches; ++t) {
    sum += a[i];
    i += stride;
    if (i >= n) {
      i -= n;
    }
  }
  return sum;
}

// TODO(unit-19): prefetch ~8 pages ahead of i (then restore if p50 does not move).
__attribute__((noinline, optimize("no-tree-vectorize"))) static std::uint64_t walk_maybe_prefetch(
    std::uint32_t const* a, int n, int stride, int touches) {
  std::uint64_t sum = 0;
  int i = 0;
  for (int t = 0; t < touches; ++t) {
    // __builtin_prefetch(a + (i + 8 * stride), 0, 3);
    sum += a[i];
    i += stride;
    if (i >= n) {
      i -= n;
    }
  }
  return sum;
}

static void* map_anon() {
  void* p = mmap(nullptr, kBytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (p == MAP_FAILED) {
    std::perror("mmap");
    std::exit(1);
  }
  return p;
}

static void prefault(void* p) {
  std::memset(p, 1, kBytes);
}

int main() {
  ll::print_header("Unit 19 — pages, TLB, prefault");
  long pg = sysconf(_SC_PAGESIZE);
  int n = n_elems();
  int page_st = page_stride_elems();
  int per_page = n / page_st;
  std::cout << "PAGE_SIZE=" << pg << "  buffer=" << (kBytes / (1024 * 1024))
            << " MiB  page stride=" << page_st << " uint32_t\n";

#ifdef MAP_HUGETLB
  void* huge = mmap(nullptr, kBytes, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
  if (huge == MAP_FAILED) {
    std::cout << "MAP_HUGETLB: failed (no reserved huge pages; typical on a VM)\n\n";
  } else {
    std::cout << "MAP_HUGETLB: ok\n\n";
    munmap(huge, kBytes);
  }
#else
  std::cout << "MAP_HUGETLB: not in this libc\n\n";
#endif

  std::vector<double> cold_ns;
  cold_ns.reserve(static_cast<std::size_t>(kSamplesCold));
  for (int r = 0; r < kSamplesCold; ++r) {
    void* p = map_anon();
    auto t0 = std::chrono::steady_clock::now();
    std::uint64_t s = walk(static_cast<std::uint32_t*>(p), n, page_st, per_page);
    auto t1 = std::chrono::steady_clock::now();
    ll::do_not_optimize(s);
    munmap(p, kBytes);
    cold_ns.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
  }
  auto cold = ll::summarize(std::move(cold_ns));
  ll::print_stats("case A (mmap untimed; first-touch in timer)", cold);

  void* warmed = map_anon();
  prefault(warmed);
  auto hot = ll::bench(
      [&] {
        std::uint64_t s =
            walk(static_cast<std::uint32_t*>(warmed), n, page_st, per_page);
        ll::do_not_optimize(s);
      },
      kSamplesHot, kWarmup);
  ll::print_stats("case B (prefault at startup, then walk)", hot);
  ll::print_speedup("cold touch", cold, "prefaulted", hot);
  munmap(warmed, kBytes);

  std::vector<std::uint32_t> resident(static_cast<std::size_t>(n), 1u);
  auto cline = ll::bench(
      [&] {
        std::uint64_t s = walk(resident.data(), n, 16, kTouches);
        ll::do_not_optimize(s);
      },
      kSamplesHot, kWarmup);
  ll::print_stats("case C (resident, stride 16)", cline);

  auto page = ll::bench(
      [&] {
        std::uint64_t s = walk(resident.data(), n, page_st, kTouches);
        ll::do_not_optimize(s);
      },
      kSamplesHot, kWarmup);
  ll::print_stats("case D (resident, page stride)", page);
  ll::print_speedup("page stride", page, "cache-line stride", cline);

  auto pref = ll::bench(
      [&] {
        std::uint64_t s = walk_maybe_prefetch(resident.data(), n, page_st, kTouches);
        ll::do_not_optimize(s);
      },
      kSamplesHot, kWarmup);
  ll::print_stats("case D' (page stride, prefetch TODO)", pref);

  void* thp = map_anon();
  madvise(thp, kBytes, MADV_HUGEPAGE);
  prefault(thp);
  auto thp_stats = ll::bench(
      [&] {
        std::uint64_t s = walk(static_cast<std::uint32_t*>(thp), n, page_st, kTouches);
        ll::do_not_optimize(s);
      },
      kSamplesHot, kWarmup);
  ll::print_stats("case E (MADV_HUGEPAGE, then walk)", thp_stats);
  ll::print_speedup("4K page stride", page, "THP page stride", thp_stats);
  munmap(thp, kBytes);

  std::cout << "\nPrefault at startup. Do not fault pages in the tick.\n";
  return 0;
}
