#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

// TODO(unit-09): try kN = 64, then 1<<20, and watch the list/vector gap.
constexpr int kN = 1 << 20;
constexpr int kSamples = 40;
constexpr int kWarmup = 4;

struct Node {
  int value;
  Node* next;
};

static std::int64_t sum_list(Node const* head) {
  std::int64_t s = 0;
  for (Node const* p = head; p; p = p->next) {
    s += p->value;
  }
  ll::do_not_optimize(s);
  return s;
}

int main() {
  ll::print_header("Unit 09 — data-oriented layout");
  std::cout << "N=" << kN << "\n\n";

  std::vector<int> vals(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    vals[static_cast<std::size_t>(i)] = i;
  }

  // Scattered heap list: one new per node (worst locality).
  Node* heap_head = nullptr;
  for (int i = kN - 1; i >= 0; --i) {
    heap_head = new Node{vals[static_cast<std::size_t>(i)], heap_head};
  }

  // Arena list: nodes are contiguous, still pointer-chasing.
  std::vector<Node> arena(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    arena[static_cast<std::size_t>(i)].value = vals[static_cast<std::size_t>(i)];
    arena[static_cast<std::size_t>(i)].next =
        (i + 1 < kN) ? &arena[static_cast<std::size_t>(i + 1)] : nullptr;
  }

  auto heap_list = ll::bench([&] { sum_list(heap_head); }, kSamples, kWarmup);
  ll::print_stats("case A (heap linked list)", heap_list);

  auto arena_list = ll::bench([&] { sum_list(&arena[0]); }, kSamples, kWarmup);
  ll::print_stats("case B (arena linked list)", arena_list);

  auto vec = ll::bench(
      [&] {
        std::int64_t s = 0;
        for (int v : vals) {
          s += v;
        }
        ll::do_not_optimize(s);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (vector of values)", vec);
  ll::print_speedup("heap list", heap_list, "vector", vec);
  ll::print_speedup("arena list", arena_list, "vector", vec);

  std::cout << "\nBig-O is the same. The cache is not.\n";

  Node* p = heap_head;
  while (p) {
    Node* n = p->next;
    delete p;
    p = n;
  }
  return 0;
}
