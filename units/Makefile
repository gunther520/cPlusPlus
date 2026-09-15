CXX      ?= g++
OPT      ?= 2
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O$(OPT) -Icommon
LDFLAGS  ?=
PTHREAD  := -pthread

BIN_DIR  := bin

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
  12-capstone-spsc-loop \
  13-syscalls-hot-path \
  14-spin-vs-sleep \
  15-simd-intrinsics \
  16-shard-per-core \
  17-ipc-shared-memory \
  18-hw-adaptive

PTHREAD_UNITS := 04-false-sharing 10-atomics-and-locks 12-capstone-spsc-loop \
  14-spin-vs-sleep 16-shard-per-core

.PHONY: all assignments clean help \
        unit-01 unit-02 unit-03 unit-04 unit-05 unit-06 \
        unit-07 unit-08 unit-09 unit-10 unit-11 unit-12 \
        unit-13 unit-14 unit-15 unit-16 unit-17 unit-18 \
        run-01 run-02 run-03 run-04 run-05 run-06 \
        run-07 run-08 run-09 run-10 run-11 run-12 \
        run-13 run-14 run-15 run-16 run-17 run-18 \
        assign-01 assign-02 assign-03 assign-04 assign-05 assign-06 \
        assign-07 assign-08 assign-09 assign-10 assign-11 assign-12 \
        assign-13 assign-14 assign-15 assign-16 assign-17 assign-18 \
        run-assign-01 run-assign-02 run-assign-03 run-assign-04 \
        run-assign-05 run-assign-06 run-assign-07 run-assign-08 \
        run-assign-09 run-assign-10 run-assign-11 run-assign-12 \
        run-assign-13 run-assign-14 run-assign-15 run-assign-16 \
        run-assign-17 run-assign-18

all: $(addprefix $(BIN_DIR)/,$(UNITS))

assignments: \
  $(BIN_DIR)/assign-01 $(BIN_DIR)/assign-02 $(BIN_DIR)/assign-03 \
  $(BIN_DIR)/assign-04 $(BIN_DIR)/assign-05 $(BIN_DIR)/assign-06 \
  $(BIN_DIR)/assign-07 $(BIN_DIR)/assign-08 $(BIN_DIR)/assign-09 \
  $(BIN_DIR)/assign-10 $(BIN_DIR)/assign-11 $(BIN_DIR)/assign-12 \
  $(BIN_DIR)/assign-13 $(BIN_DIR)/assign-14 $(BIN_DIR)/assign-15 \
  $(BIN_DIR)/assign-16 $(BIN_DIR)/assign-17 $(BIN_DIR)/assign-18

help:
	@echo "Low-latency C++ units"
	@echo "  make                 build all compare.cpp units at -O$(OPT)"
	@echo "  make unit-01         build one lesson"
	@echo "  make run-01          build and run one lesson"
	@echo "  make assignments     build all ~1h labs"
	@echo "  make run-assign-01   build and run Unit 01 lab"
	@echo "  make run-02 OPT=0    same source, no optimizer"
	@echo "  make clean           required after changing OPT"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

define UNIT_RULES
$(BIN_DIR)/$(1): $(1)/compare.cpp common/bench.hpp | $(BIN_DIR)
	$$(CXX) $$(CXXFLAGS) $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),) $$< -o $$@ $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),)

unit-$(2): $(BIN_DIR)/$(1)

run-$(2): $(BIN_DIR)/$(1)
	./$(BIN_DIR)/$(1)

$(BIN_DIR)/assign-$(2): $(1)/assignment.cpp common/bench.hpp | $(BIN_DIR)
	$$(CXX) $$(CXXFLAGS) $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),) $$< -o $$@ $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),)

assign-$(2): $(BIN_DIR)/assign-$(2)

run-assign-$(2): $(BIN_DIR)/assign-$(2)
	./$(BIN_DIR)/assign-$(2)
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
$(eval $(call UNIT_RULES,12-capstone-spsc-loop,12))
$(eval $(call UNIT_RULES,13-syscalls-hot-path,13))
$(eval $(call UNIT_RULES,14-spin-vs-sleep,14))
$(eval $(call UNIT_RULES,15-simd-intrinsics,15))
$(eval $(call UNIT_RULES,16-shard-per-core,16))
$(eval $(call UNIT_RULES,17-ipc-shared-memory,17))
$(eval $(call UNIT_RULES,18-hw-adaptive,18))

clean:
	rm -rf $(BIN_DIR)
