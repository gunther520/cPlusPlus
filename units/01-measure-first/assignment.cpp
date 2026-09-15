#include "bench.hpp"

#include <cstdint>
#include <iostream>

// Lab: Unit 01. See ASSIGNMENT.md.
// TODO: drive Case B with ll::bench (warmup + percentiles). Try kWork *= 10 and /= 10.
constexpr int kWork = 80000;

static std::uint64_t mix(int n) {
  std::uint64_t s = 0x9e3779b97f4a7c15ull;
  for (int i = 0; i < n; ++i) {
    s ^= static_cast<std::uint64_t>(i) + (s << 6) + (s >> 2);
  }
  ll::do_not_optimize(s);
  return s;
}

int main() {
  ll::print_header("Unit 01 lab");
  std::cout << "kWork=" << kWork << "\n\n";

  std::cout << "Case A: five one-shots\n";
  for (int i = 0; i < 5; ++i) {
    auto a = std::chrono::steady_clock::now();
    mix(kWork);
    auto b = std::chrono::steady_clock::now();
    std::cout << "  ";
    ll::print_ns(std::chrono::duration<double, std::nano>(b - a).count());
    std::cout << '\n';
  }

  // TODO: replace this single call with ll::bench([] { mix(kWork); }, 100, 10);
  auto t0 = std::chrono::steady_clock::now();
  mix(kWork);
  auto t1 = std::chrono::steady_clock::now();
  ll::Stats stub;
  stub.samples = 1;
  stub.min_ns = stub.p50_ns = stub.p99_ns = stub.p999_ns = stub.mean_ns =
      std::chrono::duration<double, std::nano>(t1 - t0).count();
  ll::print_stats("case B (TODO: real percentiles)", stub);

  std::cout << "\nFill the table in ASSIGNMENT.md\n";
  return 0;
}
