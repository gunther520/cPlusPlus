CXX      ?= g++
OPT      ?= 2
INCLUDES := -Icommon -I../units/common
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O$(OPT) $(INCLUDES)
PTHREAD  := -pthread

BIN_DIR := bin

.PHONY: all naive starter reference run-naive run-starter run-reference clean help

all: naive starter reference

help:
	@echo "Capstone mini matcher"
	@echo "  make naive && make run-naive"
	@echo "  make starter && make run-starter"
	@echo "  make reference && make run-reference   # spoilers"
	@echo "See ASSIGNMENT.md"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

naive: $(BIN_DIR)/naive
$(BIN_DIR)/naive: naive/main.cpp naive/engine.hpp common/harness.hpp common/order.hpp common/workload.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Inaive $(PTHREAD) naive/main.cpp -o $@ $(PTHREAD)

starter: $(BIN_DIR)/starter
$(BIN_DIR)/starter: starter/main.cpp starter/engine.hpp common/harness.hpp common/order.hpp common/workload.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Istarter $(PTHREAD) starter/main.cpp -o $@ $(PTHREAD)

reference: $(BIN_DIR)/reference
$(BIN_DIR)/reference: reference/main.cpp reference/engine.hpp common/harness.hpp common/order.hpp common/workload.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Ireference $(PTHREAD) reference/main.cpp -o $@ $(PTHREAD)

run-naive: naive
	./$(BIN_DIR)/naive

run-starter: starter
	./$(BIN_DIR)/starter

run-reference: reference
	./$(BIN_DIR)/reference

clean:
	rm -rf $(BIN_DIR)
