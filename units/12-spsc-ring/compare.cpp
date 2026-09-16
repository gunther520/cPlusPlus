#include "bench.hpp"
#include "expect.hpp"
#include "spsc.hpp"

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

// Mutex only: pre-sized slots, no heap on the tick (isolates the lock from Unit 05).
struct LockedRing {
  std::mutex mu;
  int slots[kCap]{};
  std::size_t write_pos = 0;
  std::size_t read_pos = 0;

  bool try_push(int v) {
    std::lock_guard<std::mutex> g(mu);
    if (write_pos - read_pos >= kCap) {
      return false;
    }
    slots[write_pos & (kCap - 1)] = v;
    ++write_pos;
    return true;
  }

  bool try_pop(int& v) {
    std::lock_guard<std::mutex> g(mu);
    if (read_pos == write_pos) {
      return false;
    }
    v = slots[read_pos & (kCap - 1)];
    ++read_pos;
    return true;
  }
};

using IntRing = SpscRing<int, kCap>;

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
  ll::print_header("Unit 12 — SPSC ring vs locked queue");
  std::cout << "messages=" << kMessages << "  ring cap=" << kCap << "\n\n";

  std::uint64_t const expect =
      static_cast<std::uint64_t>(kMessages) * static_cast<std::uint64_t>(kMessages - 1) / 2;

  auto locked = ll::bench(
      [&] {
        LockedQueue q;
        ll::check_eq(pump(q), expect, "locked queue sum");
      },
      kSamples, kWarmup);
  ll::print_stats("case A (mutex + std::queue)", locked);

  auto bounded = ll::bench(
      [&] {
        LockedRing q;
        ll::check_eq(pump(q), expect, "locked ring sum");
      },
      kSamples, kWarmup);
  ll::print_stats("case B (mutex + bounded ring)", bounded);
  ll::print_speedup("unbounded queue", locked, "locked ring", bounded);

  auto ring = ll::bench(
      [&] {
        IntRing q;
        ll::check_eq(pump(q), expect, "spsc sum");
      },
      kSamples, kWarmup);
  ll::print_stats("case C (SPSC ring)", ring);
  ll::print_speedup("locked ring", bounded, "SPSC", ring);

  std::cout << "\nOne producer, one consumer. Heap (A) vs lock (B) vs SPSC (C).\n";
  return 0;
}
