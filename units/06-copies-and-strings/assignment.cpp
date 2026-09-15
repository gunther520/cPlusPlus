#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

constexpr int kTok = 64;
constexpr int kN = 40000;

static std::uint64_t hash_str(std::string s) {
  std::uint64_t h = static_cast<unsigned char>(s.front()) + s.size();
  ll::do_not_optimize(h);
  return h;
}

[[maybe_unused]] static std::uint64_t hash_view(std::string_view s) {
  std::uint64_t h = static_cast<unsigned char>(s.front()) + s.size();
  ll::do_not_optimize(h);
  return h;
}

int main() {
  ll::print_header("Unit 06 lab");
  std::string tape;
  tape.reserve(static_cast<std::size_t>(kN) * static_cast<std::size_t>(kTok + 1));
  for (int i = 0; i < kN; ++i) {
    tape.append(static_cast<std::size_t>(kTok), 'x');
    tape.push_back(';');
  }

  auto byval = ll::bench(
      [&] {
        std::uint64_t h = 0;
        for (int i = 0; i < kN; ++i) {
          auto off = static_cast<std::size_t>(i) * static_cast<std::size_t>(kTok + 1);
          std::string tok = tape.substr(off, static_cast<std::size_t>(kTok));
          h ^= hash_str(tok);
        }
        ll::do_not_optimize(h);
      },
      40, 4);
  ll::print_stats("case A (substr + string by value)", byval);

  auto view = ll::bench(
      [&] {
        std::uint64_t h = 0;
        for (int i = 0; i < kN; ++i) {
          auto off = static_cast<std::size_t>(i) * static_cast<std::size_t>(kTok + 1);
          // TODO: string_view into tape; call hash_view. No substr.
          std::string tok = tape.substr(off, static_cast<std::size_t>(kTok));
          h ^= hash_str(tok);
        }
        ll::do_not_optimize(h);
      },
      40, 4);
  ll::print_stats("case B (TODO: string_view)", view);
  ll::print_speedup("by value", byval, "view", view);
  return 0;
}
