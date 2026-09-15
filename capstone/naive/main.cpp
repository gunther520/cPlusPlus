#include "engine.hpp"
#include "harness.hpp"

int main() {
  return run_engine<NaiveEngine>("Capstone naive (list + mutex + string)");
}
