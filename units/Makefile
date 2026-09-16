CXX      ?= g++
OPT      ?= 2
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O$(OPT) -Icommon
LDFLAGS  ?=
PTHREAD  := -pthread
PIN      ?= 1

BIN_DIR  := bin/O$(OPT)

UNITS := \
  01-measure-first \
  02-compiler-hot-path \
  03-cache-locality \
  04-false-sharing \
  05-allocations \
  06-copies-and-strings \
  07-branches \
  08-virtual-vs-static \
  09-data-oriented \
  10-atomics-and-locks \
  11-vectorization \
  12-spsc-ring \
  13-syscalls-hot-path \
  14-spin-vs-sleep \
  15-simd-intrinsics \
  16-shard-per-core \
  17-ipc-shared-memory \
  18-hw-adaptive \
  19-pages-tlb \
  20-exceptions \
  21-latency-budget

PTHREAD_UNITS := 04-false-sharing 10-atomics-and-locks 12-spsc-ring \
  14-spin-vs-sleep 16-shard-per-core

RUNNER := $(shell if [ "$(PIN)" != "0" ] && command -v taskset >/dev/null 2>&1 && [ "$$(nproc 2>/dev/null || echo 1)" -ge 2 ]; then echo "taskset -c 0,1"; fi)

.PHONY: all assignments clean help check tsan perf-03 \
        unit-01 unit-02 unit-03 unit-04 unit-05 unit-06 \
        unit-07 unit-08 unit-09 unit-10 unit-11 unit-12 \
        unit-13 unit-14 unit-15 unit-16 unit-17 unit-18 \
        unit-19 unit-20 unit-21 \
        run-01 run-02 run-03 run-04 run-05 run-06 \
        run-07 run-08 run-09 run-10 run-11 run-12 \
        run-13 run-14 run-15 run-16 run-17 run-18 \
        run-19 run-20 run-21 \
        assign-01 assign-02 assign-03 assign-04 assign-05 assign-06 \
        assign-07 assign-08 assign-09 assign-10 assign-11 assign-12 \
        assign-13 assign-14 assign-15 assign-16 assign-17 assign-18 \
        assign-19 assign-20 assign-21 \
        run-assign-01 run-assign-02 run-assign-03 run-assign-04 \
        run-assign-05 run-assign-06 run-assign-07 run-assign-08 \
        run-assign-09 run-assign-10 run-assign-11 run-assign-12 \
        run-assign-13 run-assign-14 run-assign-15 run-assign-16 \
        run-assign-17 run-assign-18 \
        run-assign-19 run-assign-20 run-assign-21

all: $(addprefix $(BIN_DIR)/,$(UNITS))

assignments: \
  $(BIN_DIR)/assign-01 $(BIN_DIR)/assign-02 $(BIN_DIR)/assign-03 \
  $(BIN_DIR)/assign-04 $(BIN_DIR)/assign-05 $(BIN_DIR)/assign-06 \
  $(BIN_DIR)/assign-07 $(BIN_DIR)/assign-08 $(BIN_DIR)/assign-09 \
  $(BIN_DIR)/assign-10 $(BIN_DIR)/assign-11 $(BIN_DIR)/assign-12 \
  $(BIN_DIR)/assign-13 $(BIN_DIR)/assign-14 $(BIN_DIR)/assign-15 \
  $(BIN_DIR)/assign-16 $(BIN_DIR)/assign-17 $(BIN_DIR)/assign-18 \
  $(BIN_DIR)/assign-19 $(BIN_DIR)/assign-20 $(BIN_DIR)/assign-21

help:
	@echo "Low-latency C++ units  (see TRACK.md)"
	@echo "  make                 build all compare.cpp at -O$(OPT) -> $(BIN_DIR)/"
	@echo "  make run-01          build and run one lesson (PIN=1 uses taskset)"
	@echo "  make run-02 OPT=0    same source, no optimizer (no clean needed)"
	@echo "  make check           build all + run oracles (12, capstone via root)"
	@echo "  make tsan            ThreadSanitizer on units 10 and 12"
	@echo "  make perf-03         perf stat on cache locality"
	@echo "  make assignments     build all ~1h labs"
	@echo "  PIN=0 make run-04    disable taskset"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

define UNIT_RULES
$(BIN_DIR)/$(1): $(1)/compare.cpp common/bench.hpp common/spsc.hpp common/expect.hpp | $(BIN_DIR)
	$$(CXX) $$(CXXFLAGS) $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),) $$< -o $$@ $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),)

unit-$(2): $(BIN_DIR)/$(1)

run-$(2): $(BIN_DIR)/$(1)
	$(RUNNER) ./$(BIN_DIR)/$(1)

$(BIN_DIR)/assign-$(2): $(1)/assignment.cpp common/bench.hpp common/spsc.hpp common/expect.hpp | $(BIN_DIR)
	$$(CXX) $$(CXXFLAGS) $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),) $$< -o $$@ $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),)

assign-$(2): $(BIN_DIR)/assign-$(2)

run-assign-$(2): $(BIN_DIR)/assign-$(2)
	$(RUNNER) ./$(BIN_DIR)/assign-$(2)
endef

$(eval $(call UNIT_RULES,01-measure-first,01))
$(eval $(call UNIT_RULES,02-compiler-hot-path,02))
$(eval $(call UNIT_RULES,03-cache-locality,03))
$(eval $(call UNIT_RULES,04-false-sharing,04))
$(eval $(call UNIT_RULES,05-allocations,05))
$(eval $(call UNIT_RULES,06-copies-and-strings,06))
$(eval $(call UNIT_RULES,07-branches,07))
$(eval $(call UNIT_RULES,08-virtual-vs-static,08))
$(eval $(call UNIT_RULES,09-data-oriented,09))
$(eval $(call UNIT_RULES,10-atomics-and-locks,10))
$(eval $(call UNIT_RULES,11-vectorization,11))
$(eval $(call UNIT_RULES,12-spsc-ring,12))
$(eval $(call UNIT_RULES,13-syscalls-hot-path,13))
$(eval $(call UNIT_RULES,14-spin-vs-sleep,14))
$(eval $(call UNIT_RULES,15-simd-intrinsics,15))
$(eval $(call UNIT_RULES,16-shard-per-core,16))
$(eval $(call UNIT_RULES,17-ipc-shared-memory,17))
$(eval $(call UNIT_RULES,18-hw-adaptive,18))
$(eval $(call UNIT_RULES,19-pages-tlb,19))
$(eval $(call UNIT_RULES,20-exceptions,20))
$(eval $(call UNIT_RULES,21-latency-budget,21))

check: $(BIN_DIR)/12-spsc-ring $(BIN_DIR)/01-measure-first $(BIN_DIR)/20-exceptions
	$(RUNNER) ./$(BIN_DIR)/12-spsc-ring >/tmp/u12.out
	$(RUNNER) ./$(BIN_DIR)/20-exceptions >/tmp/u20.out
	@echo "units check ok"

tsan:
	@mkdir -p bin
	$(CXX) -std=c++17 -O1 -g -fsanitize=thread -Icommon $(PTHREAD) \
	  10-atomics-and-locks/compare.cpp -o bin/u10-tsan $(PTHREAD)
	$(CXX) -std=c++17 -O1 -g -fsanitize=thread -Icommon $(PTHREAD) \
	  12-spsc-ring/compare.cpp -o bin/u12-tsan $(PTHREAD)
	$(RUNNER) ./bin/u10-tsan
	$(RUNNER) ./bin/u12-tsan

perf-03: unit-03
	@command -v perf >/dev/null 2>&1 || { echo "perf not installed (linux-tools)"; exit 1; }
	perf stat -e cache-misses,dTLB-load-misses,branch-misses $(RUNNER) ./$(BIN_DIR)/03-cache-locality

clean:
	rm -rf bin
