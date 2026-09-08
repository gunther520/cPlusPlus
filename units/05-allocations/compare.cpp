#include "bench.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

constexpr int kN = 50000;
constexpr int kSamples = 80;
constexpr int kWarmup = 5;

struct Node {
  int value;
  Node* next;
};

struct Pool {
  std::vector<Node> buf;
  std::size_t used = 0;

  explicit Pool(std::size_t n) : buf(n) {}

  Node* alloc(int value) {
    if (used >= buf.size()) {
      return nullptr;
    }
    Node* n = &buf[used++];
    n->value = value;
    n->next = nullptr;
    return n;
  }

  void reset() { used = 0; }
};

int main() {
  ll::print_header("Unit 05 — allocations");
  std::cout << "N = " << kN << "\n\n";

  auto grow = ll::bench(
      [] {
        std::vector<int> v;
        for (int i = 0; i < kN; ++i) {
          v.push_back(i);
        }
        // TODO(unit-05): uncomment the next line, then try reserve after clear.
        // v.clear(); v.shrink_to_fit();
        ll::do_not_optimize(v.data());
        ll::do_not_optimize(v.size());
      },
      kSamples, kWarmup);
  ll::print_stats("case A (push_back, no reserve)", grow);

  auto reserved = ll::bench(
      [] {
        std::vector<int> v;
        v.reserve(kN);
        for (int i = 0; i < kN; ++i) {
          v.push_back(i);
        }
        ll::do_not_optimize(v.data());
      },
      kSamples, kWarmup);
  ll::print_stats("case B (push_back + reserve)", reserved);
  ll::print_speedup("no reserve", grow, "reserve", reserved);

  std::cout << "\n";
  auto heap_nodes = ll::bench(
      [] {
        Node* head = nullptr;
        for (int i = 0; i < kN; ++i) {
          Node* n = new Node{i, head};
          head = n;
        }
        ll::do_not_optimize(head);
        while (head) {
          Node* n = head->next;
          delete head;
          head = n;
        }
      },
      40, 3);
  ll::print_stats("case C (new/delete per node)", heap_nodes);

  Pool pool(static_cast<std::size_t>(kN));
  auto pooled = ll::bench(
      [&] {
        pool.reset();
        Node* head = nullptr;
        for (int i = 0; i < kN; ++i) {
          Node* n = pool.alloc(i);
          n->next = head;
          head = n;
        }
        ll::do_not_optimize(head);
      },
      40, 3);
  ll::print_stats("case D (pre-sized node pool)", pooled);
  ll::print_speedup("new/delete", heap_nodes, "pool", pooled);

  std::cout << "\nAllocate before the event. Reuse after. Do not grow in the tick.\n";
  return 0;
}
