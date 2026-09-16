#pragma once

#include "order.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

constexpr int kOrders = 15000;
constexpr std::uint32_t kSeed = 1;
constexpr int kPxLo = 80;
constexpr int kPxHi = 180;

enum class Tape { Uniform, Cancels, OneSided };

inline Tape tape_from_env() {
  char const* t = std::getenv("LL_TAPE");
  if (t == nullptr) {
    return Tape::Uniform;
  }
  if (std::strcmp(t, "cancels") == 0) {
    return Tape::Cancels;
  }
  if (std::strcmp(t, "onesided") == 0) {
    return Tape::OneSided;
  }
  return Tape::Uniform;
}

inline char const* tape_name(Tape t) {
  switch (t) {
    case Tape::Cancels:
      return "cancels";
    case Tape::OneSided:
      return "onesided";
    default:
      return "uniform";
  }
}

inline std::vector<Order> make_workload(std::uint32_t seed = kSeed, int n = kOrders,
                                        Tape tape = tape_from_env()) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> side(0, 1);
  std::uniform_int_distribution<int> px(kPxLo, kPxHi);
  std::uniform_int_distribution<int> qty(1, 8);
  std::uniform_int_distribution<int> roll(0, 99);
  std::vector<Order> out(static_cast<std::size_t>(n));
  std::vector<std::uint32_t> live;
  live.reserve(static_cast<std::size_t>(n));

  for (int i = 0; i < n; ++i) {
    auto id = static_cast<std::uint32_t>(i + 1);
    if (tape == Tape::Cancels && !live.empty() && roll(rng) < 90) {
      std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
      std::size_t j = pick(rng);
      out[static_cast<std::size_t>(i)] =
          Order{live[j], 0, 0, 0, /*action*/ 1};
      live[j] = live.back();
      live.pop_back();
      continue;
    }

    std::uint8_t s = static_cast<std::uint8_t>(side(rng));
    std::int32_t p = px(rng);
    std::uint32_t q = static_cast<std::uint32_t>(qty(rng));
    if (tape == Tape::OneSided) {
      s = 0;
      p = kPxLo + (i % 20);
    } else if (tape == Tape::Cancels && roll(rng) < 10) {
      // ~1% of the original 100%: aggressive (crosses).
      s = static_cast<std::uint8_t>(side(rng));
      p = (s == 0) ? kPxHi : kPxLo;
    } else if (tape == Tape::Cancels) {
      // Resting add, uncrossed.
      s = static_cast<std::uint8_t>(side(rng));
      p = (s == 0) ? kPxLo + 5 : kPxHi - 5;
      live.push_back(id);
    }
    out[static_cast<std::size_t>(i)] = Order{id, s, p, q, 0};
  }
  return out;
}
