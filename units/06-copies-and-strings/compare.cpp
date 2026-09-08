#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

// TODO(unit-06): try kLong = 8, 16, 32, 64, 4096.
constexpr int kLong = 256;
constexpr int kChunks = 4000;
constexpr int kChunkLen = 64;
constexpr int kCallN = 30000;
constexpr int kSamples = 80;
constexpr int kWarmup = 8;

static std::uint64_t sink_by_value(std::string s) {
  // Touch a little of the payload so the copy cannot be proven dead,
  // without making the hash dominate the copy.
  std::uint64_t h = static_cast<unsigned char>(s.front()) +
                    static_cast<unsigned char>(s.back()) + s.size();
  ll::do_not_optimize(h);
  return h;
}

static std::uint64_t sink_by_view(std::string_view s) {
  std::uint64_t h = static_cast<unsigned char>(s.front()) +
                    static_cast<unsigned char>(s.back()) + s.size();
  ll::do_not_optimize(h);
  return h;
}

int main() {
  ll::print_header("Unit 06 — copies and strings");
  std::cout << "sizeof(std::string)=" << sizeof(std::string)
            << "  payload bytes=" << kLong << "  append chunks=" << kChunks
            << " x " << kChunkLen << "\n\n";

  std::string payload(static_cast<std::size_t>(kLong), 'x');

  auto by_val = ll::bench(
      [&] {
        std::uint64_t h = 0;
        for (int i = 0; i < kCallN; ++i) {
          h ^= sink_by_value(payload);
        }
        ll::do_not_optimize(h);
      },
      kSamples, kWarmup);
  ll::print_stats("case A (pass string by value)", by_val);

  auto by_view = ll::bench(
      [&] {
        std::uint64_t h = 0;
        for (int i = 0; i < kCallN; ++i) {
          h ^= sink_by_view(payload);
        }
        ll::do_not_optimize(h);
      },
      kSamples, kWarmup);
  ll::print_stats("case B (pass string_view)", by_view);
  ll::print_speedup("by value", by_val, "string_view", by_view);

  std::cout << "\n";
  std::string chunk(static_cast<std::size_t>(kChunkLen), 'a');
  auto append_grow = ll::bench(
      [&] {
        std::string s;
        for (int i = 0; i < kChunks; ++i) {
          s.append(chunk);
        }
        ll::do_not_optimize(s.data());
        ll::do_not_optimize(s.size());
      },
      kSamples, kWarmup);
  ll::print_stats("case C (append, no reserve)", append_grow);

  auto append_res = ll::bench(
      [&] {
        std::string s;
        s.reserve(static_cast<std::size_t>(kChunks) *
                  static_cast<std::size_t>(kChunkLen));
        for (int i = 0; i < kChunks; ++i) {
          s.append(chunk);
        }
        ll::do_not_optimize(s.data());
        ll::do_not_optimize(s.size());
      },
      kSamples, kWarmup);
  ll::print_stats("case D (append + reserve)", append_res);
  ll::print_speedup("no reserve", append_grow, "reserve", append_res);

  std::cout << "\n";
  auto sso = ll::bench(
      [] {
        std::string s("hi");
        for (int i = 0; i < kCallN; ++i) {
          s[0] = static_cast<char>('a' + (i % 26));
          ll::do_not_optimize(s.data());
        }
      },
      kSamples, kWarmup);
  ll::print_stats("case E (tiny SSO string mutate)", sso);

  auto heap_str = ll::bench(
      [] {
        std::string s(32, 'y');
        for (int i = 0; i < kCallN; ++i) {
          s[0] = static_cast<char>('a' + (i % 26));
          ll::do_not_optimize(s.data());
        }
      },
      kSamples, kWarmup);
  ll::print_stats("case F (32-byte string mutate)", heap_str);

  std::cout << "\nSSO hides the heap until the string no longer fits inline.\n";
  return 0;
}
