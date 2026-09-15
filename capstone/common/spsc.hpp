#pragma once

#include "order.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>

// Unit 12 ring, payload = Order. One producer, one consumer. Power-of-two Cap.
template <std::size_t Cap>
struct SpscOrderRing {
  static_assert((Cap & (Cap - 1)) == 0, "capacity must be a power of two");

  Order slots[Cap]{};
  alignas(64) std::atomic<std::size_t> write_pos{0};
  alignas(64) std::atomic<std::size_t> read_pos{0};

  bool try_push(Order const& v) {
    auto w = write_pos.load(std::memory_order_relaxed);
    auto r = read_pos.load(std::memory_order_acquire);
    if (w - r >= Cap) {
      return false;
    }
    slots[w & (Cap - 1)] = v;
    write_pos.store(w + 1, std::memory_order_release);
    return true;
  }

  bool try_pop(Order& v) {
    auto r = read_pos.load(std::memory_order_relaxed);
    auto w = write_pos.load(std::memory_order_acquire);
    if (r == w) {
      return false;
    }
    v = slots[r & (Cap - 1)];
    read_pos.store(r + 1, std::memory_order_release);
    return true;
  }
};
