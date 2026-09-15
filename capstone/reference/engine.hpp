#pragma once

#include "order.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

// SPOILER: a layout-friendly matcher. Read ASSIGNMENT.md before copying.
class ReferenceEngine {
  struct Resting {
    std::uint32_t id;
    std::uint32_t qty;
  };

  struct Level {
    std::vector<Resting> q;
    std::size_t head = 0;

    bool empty() const { return head >= q.size(); }
    Resting& front() { return q[head]; }

    void pop() {
      ++head;
      if (head >= q.size()) {
        q.clear();
        head = 0;
      } else if (head > 64 && head * 2 > q.size()) {
        q.erase(q.begin(), q.begin() + static_cast<std::ptrdiff_t>(head));
        head = 0;
      }
    }

    void push(Resting r) { q.push_back(r); }
  };

  static constexpr int kMaxPx = 256;
  Level bids_[kMaxPx + 1];
  Level asks_[kMaxPx + 1];
  int best_bid_ = 0;
  int best_ask_ = kMaxPx + 1;
  std::uint64_t filled_qty_ = 0;
  std::uint64_t checksum_ = 0;

  void bump_best_ask() {
    while (best_ask_ <= kMaxPx && asks_[best_ask_].empty()) {
      ++best_ask_;
    }
  }

  void bump_best_bid() {
    while (best_bid_ >= 1 && bids_[best_bid_].empty()) {
      --best_bid_;
    }
  }

  void take(std::uint32_t aggressor, Level& lvl, std::uint32_t& qty) {
    Resting& rest = lvl.front();
    std::uint32_t q = std::min(qty, rest.qty);
    filled_qty_ += q;
    checksum_ ^= mix_fill(aggressor, rest.id, q);
    qty -= q;
    rest.qty -= q;
    if (rest.qty == 0) {
      lvl.pop();
    }
  }

 public:
  ReferenceEngine() {
    for (int p = 0; p <= kMaxPx; ++p) {
      bids_[p].q.reserve(32);
      asks_[p].q.reserve(32);
    }
  }

  void on_order(Order o) {
    if (o.price < 1 || o.price > kMaxPx) {
      return;
    }
    if (o.side == 0) {
      while (o.qty > 0 && best_ask_ <= o.price) {
        if (asks_[best_ask_].empty()) {
          bump_best_ask();
          continue;
        }
        take(o.id, asks_[best_ask_], o.qty);
        if (asks_[best_ask_].empty()) {
          bump_best_ask();
        }
      }
      if (o.qty > 0) {
        bids_[o.price].push(Resting{o.id, o.qty});
        if (o.price > best_bid_) {
          best_bid_ = o.price;
        }
      }
    } else {
      while (o.qty > 0 && best_bid_ >= o.price) {
        if (bids_[best_bid_].empty()) {
          bump_best_bid();
          continue;
        }
        take(o.id, bids_[best_bid_], o.qty);
        if (bids_[best_bid_].empty()) {
          bump_best_bid();
        }
      }
      if (o.qty > 0) {
        asks_[o.price].push(Resting{o.id, o.qty});
        if (o.price < best_ask_) {
          best_ask_ = o.price;
        }
      }
    }
  }

  std::uint64_t filled_qty() const { return filled_qty_; }
  std::uint64_t checksum() const { return checksum_; }
  int resting() const {
    int n = 0;
    for (int p = 1; p <= kMaxPx; ++p) {
      n += static_cast<int>((bids_[p].q.size() - bids_[p].head) +
                            (asks_[p].q.size() - asks_[p].head));
    }
    return n;
  }
};
