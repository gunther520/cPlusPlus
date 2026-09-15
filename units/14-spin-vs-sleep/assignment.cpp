#include "bench.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

constexpr int kSpinHops = 20000;
constexpr int kSleepHops = 300;

static void ping(bool yield, bool sleep, int hops) {
  std::atomic<int> seq{0};
  std::thread a([&] {
    for (int i = 0; i < hops; ++i) {
      while ((seq.load(std::memory_order_acquire) & 1) != 0) {
        if (sleep) {
          std::this_thread::sleep_for(std::chrono::microseconds(1));
        } else if (yield) {
          std::this_thread::yield();
        }
      }
      seq.store(seq.load(std::memory_order_relaxed) + 1, std::memory_order_release);
    }
  });
  std::thread b([&] {
    for (int i = 0; i < hops; ++i) {
      while ((seq.load(std::memory_order_acquire) & 1) != 1) {
        if (sleep) {
          std::this_thread::sleep_for(std::chrono::microseconds(1));
        } else if (yield) {
          std::this_thread::yield();
        }
      }
      seq.store(seq.load(std::memory_order_relaxed) + 1, std::memory_order_release);
    }
  });
  a.join();
  b.join();
}

int main() {
  ll::print_header("Unit 14 lab");
  auto sp = ll::bench([] { ping(false, false, kSpinHops); }, 8, 1);
  ll::print_stats("case A (spin)", sp);

  auto y = ll::bench(
      [] {
        // TODO: ping(true, false, kSpinHops);
        ping(false, false, kSpinHops);
      },
      8, 1);
  ll::print_stats("case B (TODO: yield)", y);

  auto sl = ll::bench([] { ping(false, true, kSleepHops); }, 8, 1);
  ll::print_stats("case C (sleep 1us, fewer hops)", sl);
  return 0;
}
