#pragma once

#include <cstdint>

// One instrument, integer prices (ticks). side: 0 = buy, 1 = sell.
struct Order {
  std::uint32_t id;
  std::uint8_t side;
  std::int32_t price;
  std::uint32_t qty;
};

inline std::uint64_t mix_fill(std::uint32_t a, std::uint32_t b, std::uint32_t qty) {
  std::uint64_t x = (static_cast<std::uint64_t>(a) << 32) ^ b;
  x ^= static_cast<std::uint64_t>(qty) * 0x9e3779b97f4a7c15ull;
  return x;
}
