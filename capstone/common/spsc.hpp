#pragma once

#include "../../units/common/spsc.hpp"
#include "order.hpp"

template <std::size_t Cap>
using SpscOrderRing = SpscRing<Order, Cap>;
