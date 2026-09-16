#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>

namespace ll {

inline void check(bool ok, char const* msg) {
  if (!ok) {
    std::cerr << "CHECK FAILED: " << msg << '\n';
    std::exit(1);
  }
}

template <typename A, typename B>
inline void check_eq(A const& a, B const& b, char const* msg) {
  if (a != b) {
    std::cerr << "CHECK FAILED: " << msg << " (" << a << " != " << b << ")\n";
    std::exit(1);
  }
}

}  // namespace ll
