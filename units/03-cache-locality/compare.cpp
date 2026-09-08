#include "bench.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

// TODO(unit-03): try 64, 256, 1024, 2048 and watch when the row/col gap appears.
constexpr int kN = 1024;
constexpr int kSamples = 40;
constexpr int kWarmup = 4;
constexpr int kOrders = 1 << 20;

struct OrderAoS {
  std::uint32_t id;
  std::uint32_t qty;
  double price;
  std::uint64_t ts;
};

struct OrderSoA {
  std::vector<std::uint32_t> id;
  std::vector<std::uint32_t> qty;
  std::vector<double> price;
  std::vector<std::uint64_t> ts;
};

static std::int64_t walk_rows(std::vector<int> const& a, int n) {
  std::int64_t s = 0;
  for (int r = 0; r < n; ++r) {
    for (int c = 0; c < n; ++c) {
      s += a[static_cast<std::size_t>(r) * static_cast<std::size_t>(n) +
             static_cast<std::size_t>(c)];
    }
  }
  ll::do_not_optimize(s);
  return s;
}

static std::int64_t walk_cols(std::vector<int> const& a, int n) {
  std::int64_t s = 0;
  for (int c = 0; c < n; ++c) {
    for (int r = 0; r < n; ++r) {
      s += a[static_cast<std::size_t>(r) * static_cast<std::size_t>(n) +
             static_cast<std::size_t>(c)];
    }
  }
  ll::do_not_optimize(s);
  return s;
}

int main() {
  ll::print_header("Unit 03 — cache locality");
  std::cout << "matrix N=" << kN << "  ("
            << (static_cast<double>(kN) * kN * sizeof(int) / (1024.0 * 1024.0))
            << " MiB)  orders=" << kOrders << "\n\n";

  std::vector<int> mat(static_cast<std::size_t>(kN) * static_cast<std::size_t>(kN), 1);

  auto rows = ll::bench([&] { walk_rows(mat, kN); }, kSamples, kWarmup);
  ll::print_stats("case A (row-major walk)", rows);

  auto cols = ll::bench([&] { walk_cols(mat, kN); }, kSamples, kWarmup);
  ll::print_stats("case B (column-major walk)", cols);
  ll::print_speedup("column walk", cols, "row walk", rows);

  std::vector<OrderAoS> aos(kOrders);
  OrderSoA soa;
  soa.id.resize(kOrders);
  soa.qty.resize(kOrders);
  soa.price.resize(kOrders);
  soa.ts.resize(kOrders);
  for (int i = 0; i < kOrders; ++i) {
    aos[static_cast<std::size_t>(i)].price = 1.0;
    soa.price[static_cast<std::size_t>(i)] = 1.0;
  }

  std::cout << "\n";
  auto aos_stats = ll::bench(
      [&] {
        double s = 0;
        for (auto const& o : aos) {
          s += o.price;
        }
        ll::do_not_optimize(s);
      },
      30, 3);
  ll::print_stats("case C (AoS sum price)", aos_stats);

  auto soa_stats = ll::bench(
      [&] {
        double s = 0;
        for (double p : soa.price) {
          s += p;
        }
        ll::do_not_optimize(s);
      },
      30, 3);
  ll::print_stats("case D (SoA sum price)", soa_stats);
  ll::print_speedup("AoS", aos_stats, "SoA", soa_stats);

  std::cout << "\nRow walk uses sequential lines; column walk jumps N ints.\n";
  return 0;
}
