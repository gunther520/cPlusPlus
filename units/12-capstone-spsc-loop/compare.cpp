#include "bench.hpp"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

constexpr int kMessages = 400000;
constexpr int kSamples = 20;
constexpr int kWarmup = 2;
// TODO(unit-12): try kCap = 8, 1024, 1<<16.
constexpr std::size_t kCap = 1024;

struct LockedQueue {
  std::mutex mu;
  std::queue<int> q;

  bool try_push(int v) {
    std::lock_guard<std::mutex> g(mu);
    q.push(v);
    return true;
  }

  bool try_pop(int& v) {
    std::lock_guard<std::mutex> g(mu);
    if (q.empty()) {
      return false;
    }
    v = q.front();
    q.pop();
    return true;
  }
};

template <std::size_t Cap>
struct SpscRing {
  static_assert((Cap & (Cap - 1)) == 0, "capacity must be a power of two");

  int slots[Cap]{};
  alignas(64) std::atomic<std::size_t> write_pos{0};
  alignas(64) std::atomic<std::size_t> read_pos{0};

  bool try_push(int v) {
    auto w = write_pos.load(std::memory_order_relaxed);
    auto r = read_pos.load(std::memory_order_acquire);
    if (w - r >= Cap) {
      return false;
    }
    slots[w & (Cap - 1)] = v;
    write_pos.store(w + 1, std::memory_order_release);
    return true;
  }

  bool try_pop(int& v) {
    auto r = read_pos.load(std::memory_order_relaxed);
    auto w = write_pos.load(std::memory_order_acquire);
    if (r == w) {
      return false;
    }
    v = slots[r & (Cap - 1)];
    read_pos.store(r + 1, std::memory_order_release);
    return true;
  }
};

template <typename Q>
static std::uint64_t pump(Q& q) {
  std::atomic<std::uint64_t> sum{0};
  std::thread prod([&] {
    for (int i = 0; i < kMessages; ++i) {
      while (!q.try_push(i)) {
      }
    }
  });
  std::thread cons([&] {
    std::uint64_t s = 0;
    int got = 0;
    int v = 0;
    while (got < kMessages) {
      if (q.try_pop(v)) {
        s += static_cast<std::uint64_t>(v);
        ++got;
      }
    }
    sum.store(s, std::memory_order_relaxed);
  });
  prod.join();
  cons.join();
  std::uint64_t s = sum.load(std::memory_order_relaxed);
  ll::do_not_optimize(s);
  return s;
}

int main() {
  ll::print_header("Unit 12 — capstone SPSC vs locked queue");
  std::cout << "messages=" << kMessages << "  ring cap=" << kCap << "\n\n";

  auto locked = ll::bench(
      [] {
        LockedQueue q;
        pump(q);
      },
      kSamples, kWarmup);
  ll::print_stats("case A (mutex + std::queue)", locked);

  auto ring = ll::bench(
      [] {
        SpscRing<kCap> q;
        pump(q);
      },
      kSamples, kWarmup);
  ll::print_stats("case B (SPSC ring buffer)", ring);
  ll::print_speedup("locked queue", locked, "SPSC ring", ring);

  std::cout << "\nOne producer, one consumer, no heap on the tick.\n";
  return 0;
}
