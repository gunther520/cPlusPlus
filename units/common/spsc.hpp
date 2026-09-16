#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>

// One producer, one consumer. Power-of-two Cap.
// Same layout in-process (Unit 12) and in MAP_SHARED (Unit 17): place with
// mmap + new (ptr) T() so atomics start at zero.
template <typename T, std::size_t Cap>
struct SpscRing {
  static_assert((Cap & (Cap - 1)) == 0, "capacity must be a power of two");
  static_assert(std::is_trivially_copyable<T>::value, "slot type must be trivially copyable");

  T slots[Cap]{};
  alignas(64) std::atomic<std::size_t> write_pos{0};
  alignas(64) std::atomic<std::size_t> read_pos{0};

  bool try_push(T const& v) {
    auto w = write_pos.load(std::memory_order_relaxed);
    auto r = read_pos.load(std::memory_order_acquire);
    if (w - r >= Cap) {
      return false;
    }
    slots[w & (Cap - 1)] = v;
    write_pos.store(w + 1, std::memory_order_release);
    return true;
  }

  bool try_pop(T& v) {
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
