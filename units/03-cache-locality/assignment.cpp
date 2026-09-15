#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

constexpr int kN = 1 << 20;

struct Particle {
  float x, y, mass, pad;
};

int main() {
  ll::print_header("Unit 03 lab");
  std::vector<Particle> aos(static_cast<std::size_t>(kN));
  for (auto& p : aos) {
    p.x = 1.f;
  }

  auto aos_s = ll::bench(
      [&] {
        float s = 0;
        for (auto& p : aos) {
          p.x += 0.001f;
          s += p.x;
        }
        ll::do_not_optimize(s);
      },
      30, 3);
  ll::print_stats("case A (AoS touch x)", aos_s);

  // TODO: struct-of-arrays. Fill x[] and loop only x[i] += 0.001f.
  auto soa_s = aos_s;
  ll::print_stats("case B (TODO: SoA touch x)", soa_s);
  std::cout << "\nSoA should beat AoS when only x is hot.\n";
  return 0;
}
