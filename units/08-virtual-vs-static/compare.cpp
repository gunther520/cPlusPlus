#include "bench.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

constexpr int kN = 200000;
constexpr int kSamples = 80;
constexpr int kWarmup = 8;

struct Base {
  explicit Base(std::int64_t delta) : delta(delta) {}
  virtual ~Base() = default;
  virtual std::int64_t tick(std::int64_t x) const = 0;
  std::int64_t delta;
};

struct TypeA final : Base {
  using Base::Base;
  std::int64_t tick(std::int64_t x) const override { return x + delta; }
};

struct TypeB final : Base {
  using Base::Base;
  std::int64_t tick(std::int64_t x) const override { return x + delta; }
};

template <typename Derived>
struct CrtpBase {
  explicit CrtpBase(std::int64_t delta) : delta(delta) {}
  std::int64_t tick(std::int64_t x) const {
    return static_cast<Derived const*>(this)->tick_impl(x);
  }
  std::int64_t delta;
};

struct CrtpA : CrtpBase<CrtpA> {
  using CrtpBase<CrtpA>::CrtpBase;
  std::int64_t tick_impl(std::int64_t x) const { return x + delta; }
};

struct CrtpB : CrtpBase<CrtpB> {
  using CrtpBase<CrtpB>::CrtpBase;
  std::int64_t tick_impl(std::int64_t x) const { return x + delta; }
};

int main() {
  ll::print_header("Unit 08 — virtual vs static");
  std::cout << "N=" << kN << "\n\n";

  std::vector<std::unique_ptr<Base>> objs;
  objs.reserve(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    // TODO(unit-08): use only TypeA here and see if virtual Case A speeds up.
    std::int64_t d = 1 + (i & 3);
    if ((i & 1) == 0) {
      objs.emplace_back(std::make_unique<TypeA>(d));
    } else {
      objs.emplace_back(std::make_unique<TypeB>(d));
    }
  }

  std::vector<CrtpA> as;
  std::vector<CrtpB> bs;
  as.reserve(static_cast<std::size_t>(kN / 2));
  bs.reserve(static_cast<std::size_t>(kN / 2));
  for (int i = 0; i < kN; ++i) {
    std::int64_t d = 1 + (i & 3);
    if ((i & 1) == 0) {
      as.emplace_back(d);
    } else {
      bs.emplace_back(d);
    }
  }

  std::vector<std::function<std::int64_t(std::int64_t)>> fns;
  fns.reserve(static_cast<std::size_t>(kN));
  for (int i = 0; i < kN; ++i) {
    std::int64_t d = 1 + (i & 3);
    // Capture a fat object so std::function likely heap-allocates the callable.
    std::array<std::uint64_t, 8> pad{{0, 1, 2, 3, 4, 5, 6, 7}};
    fns.emplace_back([d, pad](std::int64_t x) { return x + d + static_cast<std::int64_t>(pad[0]); });
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
  ll::print_speedup("std::function", fn, "virtual", virt);

  std::cout << "\nIndirect calls block inlining. CRTP is a direct call.\n";
  return 0;
}
