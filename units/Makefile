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
  12-capstone-spsc-loop

PTHREAD_UNITS := 04-false-sharing 10-atomics-and-locks 12-capstone-spsc-loop

.PHONY: all clean help unit-01 unit-02 unit-03 unit-04 unit-05 unit-06 \
        unit-07 unit-08 unit-09 unit-10 unit-11 unit-12 \
        run-01 run-02 run-03 run-04 run-05 run-06 \
        run-07 run-08 run-09 run-10 run-11 run-12

all: $(addprefix $(BIN_DIR)/,$(UNITS))

help:
	@echo "Low-latency C++ units"
	@echo "  make              build all units at -O$(OPT)"
	@echo "  make unit-01      build one unit"
	@echo "  make run-01       build and run one unit"
	@echo "  make run-02 OPT=0 same source, no optimizer"
	@echo "  make run-02 OPT=3 same source, -O3"
	@echo "  make clean        required after changing OPT (output names ignore OPT)"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

define UNIT_RULES
$(BIN_DIR)/$(1): $(1)/compare.cpp common/bench.hpp | $(BIN_DIR)
	$$(CXX) $$(CXXFLAGS) $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),) $$< -o $$@ $(if $(filter $(1),$(PTHREAD_UNITS)),$(PTHREAD),)

unit-$(2): $(BIN_DIR)/$(1)

run-$(2): $(BIN_DIR)/$(1)
	./$(BIN_DIR)/$(1)
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

clean:
	rm -rf $(BIN_DIR)
