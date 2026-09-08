#include "bench.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

constexpr int kN = 200000;
constexpr int kSamples = 80;
constexpr int kWarmup = 8;

struct Base {
  virtual ~Base() = default;
  virtual std::int64_t tick(std::int64_t x) const = 0;
};

struct TypeA final : Base {
  std::int64_t tick(std::int64_t x) const override { return x + 1; }
};

struct TypeB final : Base {
  std::int64_t tick(std::int64_t x) const override { return x + 2; }
};

template <typename Derived>
struct CrtpBase {
  std::int64_t tick(std::int64_t x) const {
    return static_cast<Derived const*>(this)->tick_impl(x);
  }
};

struct CrtpA : CrtpBase<CrtpA> {
  std::int64_t tick_impl(std::int64_t x) const { return x + 1; }
};

struct CrtpB : CrtpBase<CrtpB> {
  std::int64_t tick_impl(std::int64_t x) const { return x + 2; }
};

int main() {
  ll::print_header("Unit 08 — virtual vs static");
  std::cout << "N=" << kN << "\n\n";

  std::vector<std::unique_ptr<Base>> objs;
  objs.reserve(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    // TODO(unit-08): use only TypeA here and see if virtual Case A speeds up.
    if ((i & 1) == 0) {
      objs.emplace_back(std::make_unique<TypeA>());
    } else {
      objs.emplace_back(std::make_unique<TypeB>());
    }
  }

  std::vector<CrtpA> as(static_cast<std::size_t>(kN / 2));
  std::vector<CrtpB> bs(static_cast<std::size_t>(kN / 2));

  std::vector<std::function<std::int64_t(std::int64_t)>> fns;
  fns.reserve(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    if ((i & 1) == 0) {
      fns.emplace_back([](std::int64_t x) { return x + 1; });
    } else {
      fns.emplace_back([](std::int64_t x) { return x + 2; });
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
      kSamples, kWarmup);
  ll::print_stats("case A (virtual, mixed types)", virt);

  auto crtp = ll::bench(
      [&] {
        std::int64_t s = 0;
        for (auto const& o : as) {
          s = o.tick(s);
        }
        for (auto const& o : bs) {
          s = o.tick(s);
        }
        ll::do_not_optimize(s);
      },
      kSamples, kWarmup);
  ll::print_stats("case B (CRTP, homogeneous loops)", crtp);
  ll::print_speedup("virtual", virt, "CRTP", crtp);

  auto fn = ll::bench(
      [&] {
        std::int64_t s = 0;
        for (auto const& f : fns) {
          s = f(s);
        }
        ll::do_not_optimize(s);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (std::function)", fn);
  ll::print_speedup("virtual", virt, "std::function", fn);

  std::cout << "\nIndirect calls block inlining. CRTP is a direct call.\n";
  return 0;
}
