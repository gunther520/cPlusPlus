#pragma once

#include "order.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

constexpr int kOrders = 15000;
constexpr std::uint32_t kSeed = 1;
constexpr int kPxLo = 80;
constexpr int kPxHi = 180;

inline std::vector<Order> make_workload(std::uint32_t seed = kSeed, int n = kOrders) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> side(0, 1);
  std::uniform_int_distribution<int> px(kPxLo, kPxHi);
  std::uniform_int_distribution<int> qty(1, 8);
  std::vector<Order> out(static_cast<std::size_t>(n));
  for (int i = 0; i < n; ++i) {
    out[static_cast<std::size_t>(i)] = Order{
        static_cast<std::uint32_t>(i + 1),
        static_cast<std::uint8_t>(side(rng)),
        px(rng),
        static_cast<std::uint32_t>(qty(rng)),
    };
  }
  return out;
}
