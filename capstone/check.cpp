#include "expect.hpp"
#include "harness.hpp"
#include "workload.hpp"

#include "../naive/engine.hpp"
#include "../reference/engine.hpp"

#include <iostream>

static void check_tape(Tape tape) {
  auto orders = make_workload(kSeed, kOrders, tape);
  auto n = replay<NaiveEngine>(orders);
  auto r = replay<ReferenceEngine>(orders);
  ll::check_eq(n.filled_qty, r.filled_qty, "filled_qty");
  ll::check_eq(n.checksum, r.checksum, "checksum");
  ll::check_eq(n.resting, r.resting, "resting");
  std::cout << "ok tape=" << tape_name(tape) << "  fills=" << n.filled_qty
            << "  checksum=" << n.checksum << "  resting=" << n.resting << '\n';
}

int main() {
  check_tape(Tape::Uniform);
  check_tape(Tape::Cancels);
  check_tape(Tape::OneSided);
  std::cout << "capstone check: naive == reference on all tapes\n";
  return 0;
}
