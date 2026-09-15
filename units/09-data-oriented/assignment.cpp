#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

constexpr int kN = 1 << 20;

struct Node {
  int value;
  Node* next;
};

int main() {
  ll::print_header("Unit 09 lab");
  std::vector<int> vals(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    vals[static_cast<std::size_t>(i)] = i;
  }

  Node* heap = nullptr;
  for (int i = kN - 1; i >= 0; --i) {
    heap = new Node{vals[static_cast<std::size_t>(i)], heap};
  }

  auto a = ll::bench(
      [&] {
        std::int64_t s = 0;
        for (Node* p = heap; p; p = p->next) {
          s += p->value;
        }
        ll::do_not_optimize(s);
      },
      25, 3);
  ll::print_stats("case A (heap list)", a);

  auto b = ll::bench(
      [&] {
        std::int64_t s = 0;
        // TODO: arena vector<Node> with next pointers, then sum
        for (Node* p = heap; p; p = p->next) {
          s += p->value;
        }
        ll::do_not_optimize(s);
      },
      25, 3);
  ll::print_stats("case B (TODO: arena list)", b);

  auto c = ll::bench(
      [&] {
        std::int64_t s = 0;
        for (int v : vals) {
          s += v;
        }
        ll::do_not_optimize(s);
      },
      25, 3);
  ll::print_stats("case C (vector values)", c);

  while (heap) {
    Node* n = heap->next;
    delete heap;
    heap = n;
  }
  return 0;
}
