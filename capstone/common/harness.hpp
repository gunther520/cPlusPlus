#pragma once

#include "bench.hpp"
#include "order.hpp"
#include "workload.hpp"

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
int run_engine(char const* name) {
  auto const orders = make_workload();
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

  ll::print_header(name);
  std::cout << "orders=" << orders.size() << "  fills_qty=" << once.filled_qty
            << "  checksum=" << once.checksum << "  resting=" << once.resting
            << "\n";
  ll::print_stats("replay whole book", stats);
  std::cout << "Write fills_qty + checksum into capstone/RESULTS.md\n";
  return 0;
}
