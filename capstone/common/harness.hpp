#pragma once

#include "bench.hpp"
#include "order.hpp"
#include "workload.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

constexpr int kCapstoneSamples = 25;
constexpr int kCapstoneWarmup = 3;

struct ReplayResult {
  std::uint64_t filled_qty = 0;
  std::uint64_t checksum = 0;
  int resting = 0;
};

template <typename Engine>
ReplayResult replay(std::vector<Order> const& orders) {
  Engine e;
  for (auto const& o : orders) {
    e.on_order(o);
  }
  ReplayResult r;
  r.filled_qty = e.filled_qty();
  r.checksum = e.checksum();
  r.resting = e.resting();
  return r;
}

template <typename Engine>
ll::Stats per_order_after_warm(std::vector<Order> const& orders) {
  int n = static_cast<int>(orders.size());
  int warm = (2 * n) / 3;
  Engine e;
  for (int i = 0; i < warm; ++i) {
    e.on_order(orders[static_cast<std::size_t>(i)]);
  }
  std::vector<double> times;
  times.reserve(static_cast<std::size_t>(n - warm));
  for (int i = warm; i < n; ++i) {
    auto t0 = std::chrono::steady_clock::now();
    e.on_order(orders[static_cast<std::size_t>(i)]);
    auto t1 = std::chrono::steady_clock::now();
    times.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
  }
  ll::do_not_optimize(e.checksum());
  return ll::summarize(std::move(times));
}

template <typename Engine>
int run_engine(char const* name) {
  Tape tape = tape_from_env();
  auto const orders = make_workload(kSeed, kOrders, tape);
  auto const once = replay<Engine>(orders);

  auto stats = ll::bench(
      [&] {
        Engine e;
        for (auto const& o : orders) {
          e.on_order(o);
        }
        ll::do_not_optimize(e.filled_qty());
        ll::do_not_optimize(e.checksum());
      },
      kCapstoneSamples, kCapstoneWarmup);

  auto per = per_order_after_warm<Engine>(orders);

  ll::print_header(name);
  std::cout << "tape=" << tape_name(tape) << "  orders=" << orders.size()
            << "  fills_qty=" << once.filled_qty << "  checksum=" << once.checksum
            << "  resting=" << once.resting << "\n";
  ll::print_stats("replay whole tape", stats);
  ll::print_stats("per-order after 2/3 warm", per);
  std::cout << "The per-order line is the tick metric. Whole-tape includes ctor.\n";
  std::cout << "Write fills_qty + checksum into capstone/RESULTS.md\n";
  std::cout << "Other tapes: LL_TAPE=cancels|onesided make run-starter\n";
  return 0;
}
