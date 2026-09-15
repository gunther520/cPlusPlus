#include "bench.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#if defined(__x86_64__) || defined(__i386__)
#include <emmintrin.h>
#define LL_PAUSE() _mm_pause()
#else
#define LL_PAUSE() ((void)0)
#endif

constexpr int kHopsSpin = 30000;
constexpr int kHopsSleep = 400;
constexpr int kSamples = 12;
constexpr int kWarmup = 1;

enum class WaitKind { Spin, Yield, Sleep };

static void ping_pong(WaitKind kind, int hops) {
  std::atomic<int> seq{0};
  std::thread prod([&] {
    for (int i = 0; i < hops; ++i) {
      while ((seq.load(std::memory_order_acquire) & 1) != 0) {
        if (kind == WaitKind::Yield) {
          std::this_thread::yield();
        } else if (kind == WaitKind::Sleep) {
          std::this_thread::sleep_for(std::chrono::microseconds(1));
        } else {
          // TODO(unit-14): uncomment LL_PAUSE();
          // LL_PAUSE();
        }
      }
      seq.store(seq.load(std::memory_order_relaxed) + 1, std::memory_order_release);
    }
  });
  std::thread cons([&] {
    for (int i = 0; i < hops; ++i) {
      while ((seq.load(std::memory_order_acquire) & 1) != 1) {
        if (kind == WaitKind::Yield) {
          std::this_thread::yield();
        } else if (kind == WaitKind::Sleep) {
          std::this_thread::sleep_for(std::chrono::microseconds(1));
        } else {
          // TODO(unit-14): uncomment LL_PAUSE();
          // LL_PAUSE();
        }
      }
      seq.store(seq.load(std::memory_order_relaxed) + 1, std::memory_order_release);
    }
  });
  prod.join();
  cons.join();
  ll::do_not_optimize(seq);
}

int main() {
  ll::print_header("Unit 14 — spin vs yield vs sleep");
  std::cout << "spin/yield hops = " << kHopsSpin
            << "  sleep hops = " << kHopsSleep << " (sleep is a syscall; fewer hops)\n\n";

  auto spin = ll::bench([] { ping_pong(WaitKind::Spin, kHopsSpin); }, kSamples, kWarmup);
  ll::print_stats("case A (busy spin)", spin);

  auto yield = ll::bench([] { ping_pong(WaitKind::Yield, kHopsSpin); }, kSamples, kWarmup);
  ll::print_stats("case B (this_thread::yield)", yield);
  ll::print_speedup("yield", yield, "spin", spin);

  auto slp = ll::bench([] { ping_pong(WaitKind::Sleep, kHopsSleep); }, kSamples, kWarmup);
  ll::print_stats("case C (sleep_for 1us)", slp);
  std::cout << "(Case C uses " << kHopsSleep << " hops, not " << kHopsSpin
            << "; do not compare those p50s as a speedup. Compare *feel*: ms vs us.)\n";

  std::cout << "\nSpin wins latency and loses the core. Sleep is a scheduler, not a clock.\n";
  return 0;
}
