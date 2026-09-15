#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

constexpr int kN = 150000;

struct Base {
  explicit Base(std::int64_t d) : delta(d) {}
  virtual ~Base() = default;
  virtual std::int64_t tick(std::int64_t x) const = 0;
  std::int64_t delta;
};
struct A final : Base {
  using Base::Base;
  std::int64_t tick(std::int64_t x) const override { return x + delta; }
};
struct B final : Base {
  using Base::Base;
  std::int64_t tick(std::int64_t x) const override { return x + delta; }
};

int main() {
  ll::print_header("Unit 08 lab");
  std::vector<std::unique_ptr<Base>> objs;
  objs.reserve(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    std::int64_t d = 1 + (i & 3);
    if (i & 1) {
      objs.emplace_back(std::make_unique<B>(d));
    } else {
      objs.emplace_back(std::make_unique<A>(d));
    }
  }

  auto virt = ll::bench(
      [&] {
        std::int64_t s = 0;
        for (auto const& o : objs) {
          s = o->tick(s);
        }
        ll::do_not_optimize(s);
      },
      50, 5);
  ll::print_stats("case A (virtual mixed)", virt);

  auto crtp = ll::bench(
      [&] {
        std::int64_t s = 0;
        // TODO: homogeneous CRTP loops or enum switch; must keep s live
        for (auto const& o : objs) {
          s = o->tick(s);
        }
        ll::do_not_optimize(s);
      },
      50, 5);
  ll::print_stats("case B (TODO: static dispatch)", crtp);
  ll::print_speedup("virtual", virt, "static", crtp);
  return 0;
}
