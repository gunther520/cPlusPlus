#include "bench.hpp"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

constexpr int kIters = 1500000;

int main() {
  ll::print_header("Unit 10 lab");
  std::mutex mu;
  std::uint64_t locked = 0;

  auto mtx = ll::bench(
      [&] {
        locked = 0;
        std::thread t1([&] {
          for (int i = 0; i < kIters; ++i) {
            std::lock_guard<std::mutex> g(mu);
            ++locked;
          }
        });
        std::thread t2([&] {
          for (int i = 0; i < kIters; ++i) {
            std::lock_guard<std::mutex> g(mu);
            ++locked;
          }
        });
        t1.join();
        t2.join();
        ll::do_not_optimize(locked);
      },
      20, 2);
  ll::print_stats("case A (mutex)", mtx);

  std::atomic<std::uint64_t> at{0};
  auto atom = ll::bench(
      [&] {
        at.store(0, std::memory_order_relaxed);
        // TODO: two threads fetch_add relaxed, like compare.cpp Case B
        for (int i = 0; i < kIters * 2; ++i) {
          at.fetch_add(1, std::memory_order_relaxed);
        }
        ll::do_not_optimize(at);
      },
      20, 2);
  ll::print_stats("case B (TODO: 2-thread atomic)", atom);

  std::uint64_t payload = 0;
  std::atomic<int> ready{0};
  auto pub = ll::bench(
      [&] {
        payload = 0;
        ready.store(0, std::memory_order_relaxed);
        std::thread prod([&] {
          payload = 0xC0FFEE;
          // TODO: memory_order_release
          ready.store(1, std::memory_order_relaxed);
        });
        std::thread cons([&] {
          // TODO: memory_order_acquire
          while (ready.load(std::memory_order_relaxed) == 0) {
          }
          ll::do_not_optimize(payload);
        });
        prod.join();
        cons.join();
      },
      30, 3);
  ll::print_stats("case C (TODO: acq/rel publish)", pub);
  return 0;
}
