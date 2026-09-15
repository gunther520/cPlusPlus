#include "bench.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

constexpr int kN = 40000;

struct Node {
  int value;
  Node* next;
};

struct Pool {
  std::vector<Node> buf;
  std::size_t used = 0;
  explicit Pool(std::size_t n) : buf(n) {}
  Node* alloc(int value) {
    // TODO: bump used, fill node, return pointer. No new.
    (void)value;
    return new Node{value, nullptr};
  }
  void reset() { used = 0; }
};

int main() {
  ll::print_header("Unit 05 lab");

  auto grow = ll::bench(
      [] {
        std::vector<int> v;
        for (int i = 0; i < kN; ++i) {
          v.push_back(i);
        }
        ll::do_not_optimize(v.data());
      },
      50, 4);
  ll::print_stats("case A (no reserve)", grow);

  auto res = ll::bench(
      [] {
        std::vector<int> v;
        // TODO: v.reserve(kN);
        for (int i = 0; i < kN; ++i) {
          v.push_back(i);
        }
        ll::do_not_optimize(v.data());
      },
      50, 4);
  ll::print_stats("case B (TODO: reserve)", res);

  auto heap = ll::bench(
      [] {
        Node* h = nullptr;
        for (int i = 0; i < kN; ++i) {
          h = new Node{i, h};
        }
        ll::do_not_optimize(h);
        while (h) {
          Node* n = h->next;
          delete h;
          h = n;
        }
      },
      20, 2);
  ll::print_stats("case C (new/delete)", heap);

  Pool pool(static_cast<std::size_t>(kN));
  auto pooled = ll::bench(
      [&] {
        pool.reset();
        Node* h = nullptr;
        for (int i = 0; i < kN; ++i) {
          Node* n = pool.alloc(i);
          n->next = h;
          h = n;
        }
        ll::do_not_optimize(h);
      },
      20, 2);
  ll::print_stats("case D (TODO: real pool)", pooled);
  return 0;
}
