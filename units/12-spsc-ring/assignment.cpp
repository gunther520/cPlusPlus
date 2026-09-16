#include "bench.hpp"
#include "expect.hpp"
#include "spsc.hpp"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

constexpr int kMessages = 200000;
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

struct StudentRing {
  // TODO: SpscRing<int, kCap> (see common/spsc.hpp). Not a mutex queue.
  LockedQueue inner;
  bool try_push(int v) { return inner.try_push(v); }
  bool try_pop(int& v) { return inner.try_pop(v); }
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
  return sum.load(std::memory_order_relaxed);
}

int main() {
  ll::print_header("Unit 12 lab");
  std::uint64_t const expect =
      static_cast<std::uint64_t>(kMessages) * static_cast<std::uint64_t>(kMessages - 1) / 2;

  auto locked = ll::bench(
      [&] {
        LockedQueue q;
        ll::check_eq(pump(q), expect, "locked sum");
      },
      12, 1);
  ll::print_stats("case A (mutex queue)", locked);

  auto ring = ll::bench(
      [&] {
        StudentRing q;
        ll::check_eq(pump(q), expect, "student ring sum");
      },
      12, 1);
  ll::print_stats("case B (TODO: SPSC)", ring);
  ll::print_speedup("locked", locked, "ring", ring);
  return 0;
}
