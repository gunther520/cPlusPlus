#include "bench.hpp"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <thread>

constexpr int kIters = 20000000;
constexpr int kSamples = 25;
constexpr int kWarmup = 2;

struct Adjacent {
  std::atomic<std::uint64_t> a{0};
  std::atomic<std::uint64_t> b{0};
};

struct Padded {
  alignas(64) std::atomic<std::uint64_t> a{0};
  // TODO(unit-04): change 64 -> 32 and see if false sharing returns.
  alignas(64) std::atomic<std::uint64_t> b{0};
};

template <typename Pair>
static void hammer_pair(Pair& p) {
  p.a.store(0, std::memory_order_relaxed);
  p.b.store(0, std::memory_order_relaxed);
  std::thread t1([&] {
    for (int i = 0; i < kIters; ++i) {
      p.a.fetch_add(1, std::memory_order_relaxed);
    }
  });
  std::thread t2([&] {
    for (int i = 0; i < kIters; ++i) {
      p.b.fetch_add(1, std::memory_order_relaxed);
    }
  });
  t1.join();
  t2.join();
  ll::do_not_optimize(p.a);
  ll::do_not_optimize(p.b);
}

int main() {
  ll::print_header("Unit 04 — false sharing");
  std::cout << "iters/thread = " << kIters
            << "  sizeof(Adjacent)=" << sizeof(Adjacent)
            << "  sizeof(Padded)=" << sizeof(Padded) << "\n\n";

  Adjacent adj;
  Padded pad;

  auto false_share = ll::bench([&] { hammer_pair(adj); }, kSamples, kWarmup);
  ll::print_stats("case A (adjacent atomics)", false_share);

  auto isolated = ll::bench([&] { hammer_pair(pad); }, kSamples, kWarmup);
  ll::print_stats("case B (alignas(64) each)", isolated);
  ll::print_speedup("adjacent", false_share, "padded", isolated);

  auto one_thread = ll::bench(
      [&] {
        pad.a.store(0, std::memory_order_relaxed);
        for (int i = 0; i < kIters; ++i) {
          pad.a.fetch_add(1, std::memory_order_relaxed);
        }
        ll::do_not_optimize(pad.a);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (one thread, padded a)", one_thread);

  std::cout << "\nCase A is slow because two cores bounce one 64-byte line.\n";
  return 0;
}
