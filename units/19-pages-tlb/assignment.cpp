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

constexpr std::size_t kBytes = 32ull << 20;
constexpr int kTouches = 1 << 17;

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

static void* map_anon() {
  void* p = mmap(nullptr, kBytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (p == MAP_FAILED) {
    std::perror("mmap");
    std::exit(1);
  }
  return p;
}

int main() {
  ll::print_header("Unit 19 lab");
  int n = n_elems();
  int page_st = page_stride_elems();
  int per_page = n / page_st;

  std::vector<double> cold_ns;
  for (int r = 0; r < 6; ++r) {
    void* p = map_anon();
    auto t0 = std::chrono::steady_clock::now();
    std::uint64_t s = walk(static_cast<std::uint32_t*>(p), n, page_st, per_page);
    auto t1 = std::chrono::steady_clock::now();
    ll::do_not_optimize(s);
    munmap(p, kBytes);
    cold_ns.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
  }
  auto cold = ll::summarize(std::move(cold_ns));
  ll::print_stats("case A (first-touch in timer)", cold);

  std::vector<double> hot_ns;
  for (int r = 0; r < 6; ++r) {
    void* p = map_anon();
    auto t0 = std::chrono::steady_clock::now();
    // TODO: prefault *before* t0 (memset every byte, or store to each page).
    std::uint64_t s = walk(static_cast<std::uint32_t*>(p), n, page_st, per_page);
    auto t1 = std::chrono::steady_clock::now();
    ll::do_not_optimize(s);
    munmap(p, kBytes);
    hot_ns.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
  }
  auto hot = ll::summarize(std::move(hot_ns));
  ll::print_stats("case B (TODO: prefault, then walk)", hot);
  ll::print_speedup("cold", cold, "prefault", hot);

  std::vector<std::uint32_t> resident(static_cast<std::size_t>(n), 1u);
  auto cline = ll::bench(
      [&] {
        std::uint64_t s = walk(resident.data(), n, 16, kTouches);
        ll::do_not_optimize(s);
      },
      16, 2);
  ll::print_stats("case C (stride 16)", cline);

  auto page = ll::bench(
      [&] {
        // TODO: use page_st instead of 16
        std::uint64_t s = walk(resident.data(), n, 16, kTouches);
        ll::do_not_optimize(s);
      },
      16, 2);
  ll::print_stats("case D (TODO: page stride)", page);
  ll::print_speedup("page", page, "cache-line", cline);
  return 0;
}
