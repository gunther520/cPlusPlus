CXX      ?= g++
OPT      ?= 2
INCLUDES := -Icommon -I../units/common
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O$(OPT) $(INCLUDES)
PTHREAD  := -pthread

BIN_DIR := bin/O$(OPT)

.PHONY: all naive starter reference check run-naive run-starter run-reference clean help

all: naive starter reference

help:
	@echo "Capstone mini matcher"
	@echo "  make naive && make run-naive"
	@echo "  make starter && make run-starter"
	@echo "  make reference && make run-reference   # spoilers"
	@echo "  make check                             # naive == reference on all tapes"
	@echo "  LL_TAPE=cancels make run-naive"
	@echo "See ASSIGNMENT.md and ../docs/ARCHITECTURE.md"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

naive: $(BIN_DIR)/naive
$(BIN_DIR)/naive: naive/main.cpp naive/engine.hpp common/harness.hpp common/order.hpp common/workload.hpp common/risk.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Inaive $(PTHREAD) naive/main.cpp -o $@ $(PTHREAD)

starter: $(BIN_DIR)/starter
$(BIN_DIR)/starter: starter/main.cpp starter/engine.hpp common/harness.hpp common/order.hpp common/workload.hpp common/risk.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Istarter $(PTHREAD) starter/main.cpp -o $@ $(PTHREAD)

reference: $(BIN_DIR)/reference
$(BIN_DIR)/reference: reference/main.cpp reference/engine.hpp common/harness.hpp common/order.hpp common/workload.hpp common/risk.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Ireference $(PTHREAD) reference/main.cpp -o $@ $(PTHREAD)

check: $(BIN_DIR)/check
	./$(BIN_DIR)/check

$(BIN_DIR)/check: check.cpp naive/engine.hpp reference/engine.hpp common/harness.hpp common/workload.hpp common/order.hpp common/risk.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Inaive -Ireference check.cpp -o $@

run-naive: naive
	./$(BIN_DIR)/naive

run-starter: starter
	./$(BIN_DIR)/starter

run-reference: reference
	./$(BIN_DIR)/reference

clean:
	rm -rf bin
