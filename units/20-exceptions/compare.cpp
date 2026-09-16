#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>

constexpr int kN = 200000;
constexpr int kSamples = 40;
constexpr int kWarmup = 4;

struct Fail {
  int code;
};

__attribute__((noinline)) static int add_ec(int a, int b, int* err, int bad) {
  if (b == bad) {
    *err = 1;
    return 0;
  }
  *err = 0;
  return a + b;
}

__attribute__((noinline)) static int add_ex(int a, int b, int bad) {
  if (b == bad) {
    // TODO(unit-20): throw std::runtime_error("fail"); instead of Fail.
    throw Fail{1};
  }
  return a + b;
}

int main() {
  ll::print_header("Unit 20 — exceptions vs error codes");
  std::cout << "N=" << kN << " calls per sample\n\n";

  auto ec0 = ll::bench(
      [&] {
        int sum = 0;
        int err = 0;
        for (int i = 0; i < kN; ++i) {
          sum += add_ec(1, 0, &err, 1);
        }
        ll::do_not_optimize(sum);
        ll::do_not_optimize(err);
      },
      kSamples, kWarmup);
  ll::print_stats("case A (error code, 0% fail)", ec0);

  auto ex0 = ll::bench(
      [&] {
        int sum = 0;
        for (int i = 0; i < kN; ++i) {
          try {
            sum += add_ex(1, 0, 1);
          } catch (Fail const&) {
            sum += 0;
          }
        }
        ll::do_not_optimize(sum);
      },
      kSamples, kWarmup);
  ll::print_stats("case B (exceptions, 0% fail)", ex0);
  ll::print_speedup("ex 0%", ex0, "ec 0%", ec0);

  auto ec1 = ll::bench(
      [&] {
        int sum = 0;
        int err = 0;
        for (int i = 0; i < kN; ++i) {
          int b = (i % 100 == 0) ? 1 : 0;
          sum += add_ec(1, b, &err, 1);
          if (err) {
            sum += 0;
          }
        }
        ll::do_not_optimize(sum);
      },
      kSamples, kWarmup);
  ll::print_stats("case C (error code, 1% fail)", ec1);

  auto ex1 = ll::bench(
      [&] {
        int sum = 0;
        for (int i = 0; i < kN; ++i) {
          int b = (i % 100 == 0) ? 1 : 0;
          try {
            sum += add_ex(1, b, 1);
          } catch (Fail const&) {
            sum += 0;
          } catch (std::runtime_error const&) {
            sum += 0;
          }
        }
        ll::do_not_optimize(sum);
      },
      kSamples, kWarmup);
  ll::print_stats("case D (exceptions, 1% fail)", ex1);
  ll::print_speedup("exceptions 1%", ex1, "error codes 1%", ec1);

  std::cout << "\nDo not throw on the tick. Return a code (or do not fail).\n";
  return 0;
}
