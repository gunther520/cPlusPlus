#include "bench.hpp"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

constexpr int kN = 15000;

int main() {
  ll::print_header("Unit 13 lab");
  FILE* n = std::fopen("/dev/null", "w");
  if (!n) {
    return 1;
  }
  std::setvbuf(n, nullptr, _IONBF, 0);

  auto io = ll::bench(
      [&] {
        for (int i = 0; i < kN; ++i) {
          std::fprintf(n, "id=%d\n", i);
        }
      },
      15, 2);
  ll::print_stats("case A (fprintf /dev/null)", io);

  auto mem = ll::bench(
      [] {
        std::string s;
        // TODO: s.reserve(...)
        for (int i = 0; i < kN; ++i) {
          s.append("id=");
          s.append(std::to_string(i));
          s.push_back('\n');
        }
        ll::do_not_optimize(s.data());
      },
      15, 2);
  ll::print_stats("case B (TODO: reserve)", mem);
  ll::print_speedup("fprintf", io, "memory", mem);
  std::fclose(n);
  return 0;
}
