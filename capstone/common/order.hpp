#pragma once

#include <cstdint>

// One instrument, integer prices (ticks). side: 0 = buy, 1 = sell.
// action: 0 = new order, 1 = cancel the resting order with this id.
// Legal add prices are [kMinPx, kMaxPx] — same contract for naive and reference.
constexpr int kMinPx = 1;
constexpr int kMaxPx = 256;

struct Order {
  std::uint32_t id;
  std::uint8_t side;
  std::int32_t price;
  std::uint32_t qty;
  std::uint8_t action = 0;
};

inline std::uint64_t mix_fill(std::uint32_t aggressor, std::uint32_t rest,
                              std::int32_t px, std::uint32_t qty) {
  std::uint64_t x = (static_cast<std::uint64_t>(aggressor) << 32) ^ rest;
  x ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(px)) * 0x9e3779b97f4a7c15ull;
  x ^= static_cast<std::uint64_t>(qty) * 0xbf58476d1ce4e5b9ull;
  return x;
}
