#include "bench.hpp"

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
  // TODO: power-of-two slots, alignas(64) head/tail, try_push/try_pop like Unit 12.
  LockedQueue inner;
  bool try_push(int v) { return inner.try_push(v); }
  bool try_pop(int& v) { return inner.try_pop(v); }
};

template <typename Q>
static void pump(Q& q) {
  std::thread prod([&] {
    for (int i = 0; i < kMessages; ++i) {
      while (!q.try_push(i)) {
      }
    }
  });
  std::thread cons([&] {
    int got = 0;
    int v = 0;
    std::uint64_t s = 0;
    while (got < kMessages) {
      if (q.try_pop(v)) {
        s += static_cast<std::uint64_t>(v);
        ++got;
      }
    }
    ll::do_not_optimize(s);
  });
  prod.join();
  cons.join();
}

int main() {
  ll::print_header("Unit 12 lab");
  auto locked = ll::bench(
      [] {
        LockedQueue q;
        pump(q);
      },
      12, 1);
  ll::print_stats("case A (mutex queue)", locked);

  auto ring = ll::bench(
      [] {
        StudentRing q;
        pump(q);
      },
      12, 1);
  ll::print_stats("case B (TODO: SPSC)", ring);
  ll::print_speedup("locked", locked, "ring", ring);
  return 0;
}
