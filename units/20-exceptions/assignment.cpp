#include "bench.hpp"

#include <iostream>

constexpr int kN = 200000;

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
    throw Fail{1};
  }
  return a + b;
}

int main() {
  ll::print_header("Unit 20 lab");

  auto ec = ll::bench(
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
      30, 3);
  ll::print_stats("case A (error code, 1% fail)", ec);

  auto ex = ll::bench(
      [&] {
        int sum = 0;
        for (int i = 0; i < kN; ++i) {
          int b = (i % 100 == 0) ? 1 : 0;
          // TODO: try { sum += add_ex(1, b, 1); } catch (Fail const&) { sum += 0; }
          int err = 0;
          sum += add_ec(1, b, &err, 1);
          if (err) {
            sum += 0;
          }
        }
        ll::do_not_optimize(sum);
      },
      30, 3);
  ll::print_stats("case B (TODO: exceptions, 1% fail)", ex);
  ll::print_speedup("ec", ec, "ex", ex);
  (void)add_ex;
  return 0;
}
