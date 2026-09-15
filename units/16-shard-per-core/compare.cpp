#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr int kEvents = 200000;
constexpr int kShards = 4;
constexpr int kSamples = 20;
constexpr int kWarmup = 2;
constexpr int kKeys = 4096;

struct Event {
  std::uint32_t key;
  std::uint32_t delta;
};

static std::vector<Event> make_events() {
  std::vector<Event> ev(static_cast<std::size_t>(kEvents));
  for (int i = 0; i < kEvents; ++i) {
    ev[static_cast<std::size_t>(i)].key = static_cast<std::uint32_t>(i % kKeys);
    ev[static_cast<std::size_t>(i)].delta = 1;
  }
  return ev;
}

static std::uint64_t run_locked(std::vector<Event> const& ev, int nthreads) {
  std::mutex mu;
  std::unordered_map<std::uint32_t, std::uint64_t> map;
  map.reserve(static_cast<std::size_t>(kKeys));
  std::vector<std::thread> ts;
  ts.reserve(static_cast<std::size_t>(nthreads));
  auto worker = [&](int tid) {
    for (int i = tid; i < kEvents; i += nthreads) {
      Event const& e = ev[static_cast<std::size_t>(i)];
      std::lock_guard<std::mutex> g(mu);
      map[e.key] += e.delta;
    }
  };
  for (int t = 0; t < nthreads; ++t) {
    ts.emplace_back(worker, t);
  }
  for (auto& th : ts) {
    th.join();
  }
  std::uint64_t s = 0;
  for (auto const& kv : map) {
    s += kv.second;
  }
  ll::do_not_optimize(s);
  return s;
}

static std::uint64_t run_sharded(std::vector<Event> const& ev) {
  std::unordered_map<std::uint32_t, std::uint64_t> maps[kShards];
  for (int s = 0; s < kShards; ++s) {
    maps[s].reserve(static_cast<std::size_t>(kKeys / kShards + 8));
  }
  std::vector<std::thread> ts;
  ts.reserve(static_cast<std::size_t>(kShards));
  auto worker = [&](int shard) {
    // TODO(unit-16): use `1` instead of kShards to collapse all keys onto shard 0.
    int const nsh = kShards;
    for (int i = 0; i < kEvents; ++i) {
      Event const& e = ev[static_cast<std::size_t>(i)];
      if (static_cast<int>(e.key % static_cast<std::uint32_t>(nsh)) != shard) {
        continue;
      }
      maps[shard][e.key] += e.delta;
    }
  };
  for (int t = 0; t < kShards; ++t) {
    ts.emplace_back(worker, t);
  }
  for (auto& th : ts) {
    th.join();
  }
  std::uint64_t s = 0;
  for (int i = 0; i < kShards; ++i) {
    for (auto const& kv : maps[i]) {
      s += kv.second;
    }
  }
  ll::do_not_optimize(s);
  return s;
}

int main() {
  ll::print_header("Unit 16 — shard-per-core");
  auto ev = make_events();
  std::cout << "events=" << kEvents << "  shards=" << kShards << "\n\n";

  auto one = ll::bench([&] { run_locked(ev, 1); }, kSamples, kWarmup);
  ll::print_stats("case A (1 thread, mutex map)", one);

  auto locked4 = ll::bench([&] { run_locked(ev, 4); }, kSamples, kWarmup);
  ll::print_stats("case B (4 threads, one mutex map)", locked4);
  ll::print_speedup("4-thread locked", locked4, "1-thread locked", one);

  auto sharded = ll::bench([&] { run_sharded(ev); }, kSamples, kWarmup);
  ll::print_stats("case C (4 shards, no shared lock)", sharded);
  ll::print_speedup("4-thread locked", locked4, "sharded", sharded);

  std::cout << "\nMore threads help only if they do not fight over one map.\n";
  return 0;
}
