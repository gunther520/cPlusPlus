#include "bench.hpp"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <thread>

constexpr int kIters = 8000000;
constexpr int kSamples = 20;

struct Adjacent {
  std::atomic<std::uint64_t> a{0};
  std::atomic<std::uint64_t> b{0};
};

struct Padded {
  // TODO: alignas(64) on a and b (or insert padding).
  std::atomic<std::uint64_t> a{0};
  std::atomic<std::uint64_t> b{0};
};

template <typename Pair>
static void hammer(Pair& p) {
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
}

int main() {
  ll::print_header("Unit 04 lab");
  Adjacent adj;
  Padded pad;
  std::cout << "sizeof Adjacent=" << sizeof(Adjacent) << "  Padded=" << sizeof(Padded)
            << "\n\n";

  auto a = ll::bench([&] { hammer(adj); }, kSamples, 2);
  ll::print_stats("case A (adjacent)", a);
  auto b = ll::bench([&] { hammer(pad); }, kSamples, 2);
  ll::print_stats("case B (TODO: pad)", b);
  ll::print_speedup("adjacent", a, "padded", b);
  return 0;
}
