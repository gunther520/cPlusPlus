#pragma once

#include "order.hpp"

// Precomputed limits. Call before matching. No map, no alloc.
struct Risk {
  bool allow(Order const& o) const {
    if (o.action != 0) {
      return true;
    }
    return o.qty > 0 && o.price >= kMinPx && o.price <= kMaxPx;
  }
};
