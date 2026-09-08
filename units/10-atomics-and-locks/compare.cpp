#include "bench.hpp"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

constexpr int kIters = 2000000;
constexpr int kSamples = 30;
constexpr int kWarmup = 3;

int main() {
  ll::print_header("Unit 10 — atomics and locks");
  std::cout << "iters/thread = " << kIters << "\n\n";

  std::mutex mu;
  std::uint64_t locked = 0;
  auto mutex_stats = ll::bench(
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
      kSamples, kWarmup);
  ll::print_stats("case A (mutex ++, 2 threads)", mutex_stats);

  std::atomic<std::uint64_t> rel{0};
  auto relaxed = ll::bench(
      [&] {
        rel.store(0, std::memory_order_relaxed);
        std::thread t1([&] {
          for (int i = 0; i < kIters; ++i) {
            rel.fetch_add(1, std::memory_order_relaxed);
          }
        });
        std::thread t2([&] {
          for (int i = 0; i < kIters; ++i) {
            rel.fetch_add(1, std::memory_order_relaxed);
          }
        });
        t1.join();
        t2.join();
        ll::do_not_optimize(rel);
      },
      kSamples, kWarmup);
  ll::print_stats("case B (atomic relaxed, 2 threads)", relaxed);
  ll::print_speedup("mutex", mutex_stats, "atomic relaxed", relaxed);

  std::atomic<std::uint64_t> sc{0};
  auto seqcst = ll::bench(
      [&] {
        sc.store(0);
        std::thread t1([&] {
          for (int i = 0; i < kIters; ++i) {
            sc.fetch_add(1, std::memory_order_seq_cst);
          }
        });
        std::thread t2([&] {
          for (int i = 0; i < kIters; ++i) {
            sc.fetch_add(1, std::memory_order_seq_cst);
          }
        });
        t1.join();
        t2.join();
        ll::do_not_optimize(sc);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (atomic seq_cst, 2 threads)", seqcst);
  ll::print_speedup("seq_cst", seqcst, "relaxed", relaxed);

  std::uint64_t one = 0;
  auto uncontended = ll::bench(
      [&] {
        one = 0;
        for (int i = 0; i < kIters * 2; ++i) {
          std::lock_guard<std::mutex> g(mu);
          ++one;
        }
        ll::do_not_optimize(one);
      },
      kSamples, kWarmup);
  ll::print_stats("case D (uncontended mutex, 1 thread)", uncontended);

  // Publication example: payload then flag. Correct orders: release/acquire.
  // TODO(unit-10): change release/acquire to relaxed and reason about visibility.
  std::uint64_t payload = 0;
  std::atomic<int> ready{0};
  auto publish = ll::bench(
      [&] {
        payload = 0;
        ready.store(0, std::memory_order_relaxed);
        std::thread prod([&] {
          payload = 0xC0FFEE;
          ready.store(1, std::memory_order_release);
        });
        std::thread cons([&] {
          while (ready.load(std::memory_order_acquire) == 0) {
          }
          ll::do_not_optimize(payload);
        });
        prod.join();
        cons.join();
      },
      kSamples, kWarmup);
  ll::print_stats("case E (payload + acq/rel flag)", publish);

  std::cout << "\nRelaxed is for counters. Acquire/release is for messages.\n";
  return 0;
}
