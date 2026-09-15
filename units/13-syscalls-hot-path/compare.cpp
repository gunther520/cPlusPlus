#include "bench.hpp"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

constexpr int kN = 20000;
constexpr int kSamples = 20;
constexpr int kWarmup = 2;

// TODO(unit-13): in Case B, drop reserve and watch p99.
static void log_text(std::string& s, int i) {
  s.append("id=");
  s.append(std::to_string(i));
  s.push_back('\n');
}

int main() {
  ll::print_header("Unit 13 — syscalls on the hot path");
  std::cout << "events = " << kN << "  (Case A writes unbuffered to /dev/null)\n\n";

  FILE* devnull = std::fopen("/dev/null", "w");
  if (devnull == nullptr) {
    std::perror("fopen /dev/null");
    return 1;
  }
  std::setvbuf(devnull, nullptr, _IONBF, 0);

  auto with_io = ll::bench(
      [&] {
        std::uint64_t sink = 0;
        for (int i = 0; i < kN; ++i) {
          std::fprintf(devnull, "id=%d\n", i);
          sink += static_cast<std::uint64_t>(i);
        }
        ll::do_not_optimize(sink);
      },
      kSamples, kWarmup);
  ll::print_stats("case A (fprintf /dev/null, unbuffered)", with_io);

  auto buffered = ll::bench(
      [] {
        std::string s;
        s.reserve(static_cast<std::size_t>(kN) * 16u);
        for (int i = 0; i < kN; ++i) {
          log_text(s, i);
        }
        ll::do_not_optimize(s.data());
        ll::do_not_optimize(s.size());
      },
      kSamples, kWarmup);
  ll::print_stats("case B (reserve + append, no write)", buffered);
  ll::print_speedup("fprintf", with_io, "in-memory log", buffered);

  auto silent = ll::bench(
      [] {
        std::uint64_t sink = 0x9e3779b97f4a7c15ull;
        for (int i = 0; i < kN; ++i) {
          sink ^= static_cast<std::uint64_t>(i) + (sink << 6) + (sink >> 2);
        }
        ll::do_not_optimize(sink);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (checksum only, no log)", silent);
  ll::print_speedup("fprintf", with_io, "no log", silent);

  std::fclose(devnull);
  std::cout << "\nFormatting + write(2) are not 'free debugging'.\n";
  return 0;
}
