#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr int kEvents = 120000;
constexpr int kShards = 4;
constexpr int kKeys = 2048;

struct Event {
  std::uint32_t key;
  std::uint32_t delta;
};

int main() {
  ll::print_header("Unit 16 lab");
  std::vector<Event> ev(static_cast<std::size_t>(kEvents));
  for (int i = 0; i < kEvents; ++i) {
    ev[static_cast<std::size_t>(i)] = Event{static_cast<std::uint32_t>(i % kKeys), 1};
  }

  auto locked4 = ll::bench(
      [&] {
        std::mutex mu;
        std::unordered_map<std::uint32_t, std::uint64_t> map;
        map.reserve(static_cast<std::size_t>(kKeys));
        std::vector<std::thread> ts;
        for (int t = 0; t < kShards; ++t) {
          ts.emplace_back([&, t] {
            for (int i = t; i < kEvents; i += kShards) {
              std::lock_guard<std::mutex> g(mu);
              map[ev[static_cast<std::size_t>(i)].key] += 1;
            }
          });
        }
        for (auto& th : ts) {
          th.join();
        }
        ll::do_not_optimize(map.size());
      },
      12, 1);
  ll::print_stats("case A (4 threads, one mutex map)", locked4);

  auto sharded = ll::bench(
      [&] {
        // TODO: maps[kShards], no mutex, each thread owns key % kShards
        std::mutex mu;
        std::unordered_map<std::uint32_t, std::uint64_t> map;
        map.reserve(static_cast<std::size_t>(kKeys));
        std::vector<std::thread> ts;
        for (int t = 0; t < kShards; ++t) {
          ts.emplace_back([&, t] {
            for (int i = t; i < kEvents; i += kShards) {
              std::lock_guard<std::mutex> g(mu);
              map[ev[static_cast<std::size_t>(i)].key] += 1;
            }
          });
        }
        for (auto& th : ts) {
          th.join();
        }
        ll::do_not_optimize(map.size());
      },
      12, 1);
  ll::print_stats("case B (TODO: shard maps)", sharded);
  ll::print_speedup("locked4", locked4, "sharded", sharded);
  return 0;
}
