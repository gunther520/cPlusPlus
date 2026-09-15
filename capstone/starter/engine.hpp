#pragma once

#include "order.hpp"

#include <algorithm>
#include <cstdint>
#include <list>
#include <mutex>
#include <string>

// Same matching rules as NaiveEngine. Replace the internals.
// Keep filled_qty / checksum / resting working the same way (mix_fill).
//
// TODO(capstone):
//   1. Delete std::string and the mutex (Units 06, 10).
//   2. Replace std::list with price-indexed vectors (Units 03, 09).
//   3. reserve/pool so on_order does not allocate (Unit 05).
//   4. Track best bid / best ask; do not scan the whole book.
//   5. Stretch: SpscOrderRing (common/spsc.hpp) + generator thread (Units 12, 14).

class StudentEngine {
  struct Resting {
    std::uint32_t id;
    std::string side;
    std::int32_t price;
    std::uint32_t qty;
  };

  std::mutex mu_;
  std::list<Resting> bids_;
  std::list<Resting> asks_;
  std::uint64_t filled_qty_ = 0;
  std::uint64_t checksum_ = 0;

  void fill(std::uint32_t aggressor, Resting& rest, std::uint32_t qty) {
    filled_qty_ += qty;
    checksum_ ^= mix_fill(aggressor, rest.id, qty);
    rest.qty -= qty;
  }

 public:
  void on_order(Order o) {
    std::lock_guard<std::mutex> g(mu_);
    if (o.side == 0) {
      while (o.qty > 0) {
        auto best = asks_.end();
        for (auto it = asks_.begin(); it != asks_.end(); ++it) {
          if (it->price <= o.price &&
              (best == asks_.end() || it->price < best->price)) {
            best = it;
          }
        }
        if (best == asks_.end()) {
          break;
        }
        std::uint32_t q = std::min(o.qty, best->qty);
        fill(o.id, *best, q);
        o.qty -= q;
        if (best->qty == 0) {
          asks_.erase(best);
        }
      }
      if (o.qty > 0) {
        bids_.push_back(Resting{o.id, std::string("buy"), o.price, o.qty});
      }
    } else {
      while (o.qty > 0) {
        auto best = bids_.end();
        for (auto it = bids_.begin(); it != bids_.end(); ++it) {
          if (it->price >= o.price &&
              (best == bids_.end() || it->price > best->price)) {
            best = it;
          }
        }
        if (best == bids_.end()) {
          break;
        }
        std::uint32_t q = std::min(o.qty, best->qty);
        fill(o.id, *best, q);
        o.qty -= q;
        if (best->qty == 0) {
          bids_.erase(best);
        }
      }
      if (o.qty > 0) {
        asks_.push_back(Resting{o.id, std::string("sell"), o.price, o.qty});
      }
    }
  }

  std::uint64_t filled_qty() const { return filled_qty_; }
  std::uint64_t checksum() const { return checksum_; }
  int resting() const {
    return static_cast<int>(bids_.size() + asks_.size());
  }
};
